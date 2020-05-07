#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "GameClient.h"
#include "Jwt.h"
#include "Protocol.h"
#include "utilities.h"
#include "json.hpp"
#include "debug.h"
#include "globals.h"

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const std::string jwt_key = "bjk1903";

Requests::Requests(int sock) {
    _socket = sock;
}

Requests::~Requests() {
    mlog.log_debug("request destructor");
}

void Requests::send_request(RequestCodes code, string data) {
    set_header(REQUEST_HEADER);
    set_request_code(code);
    if (code != REQ_FB_LOGIN)
        add_data(g_user_info.token);
    add_data(data);
    _out_packet.set_crc();
    _out_packet.send_packet(_socket);
}

void Requests::send_response() {
    _out_packet.send_packet(_socket);
}

int Requests::get_socket() {
    return _socket;
}

bool Requests::get_response(string buff) {
    string indata = "";
    RequestCodes req_code;
    ErrorCodes err;
    
    if (!_in_packet.receive_packet(buff)) {
        cerr << "ERROR RECEIVE PACKET" << endl;
        close(_socket);
        return false;
    }
    
    if (!_in_packet.check_crc()) {
        mlog.log_error("crc error");
        close(_socket);
        return false;
    }
    
    req_code = (RequestCodes)_in_packet.get_request_code();
    indata = _in_packet.get_data();
    
    _in_packet.free_buffer(); // i wont use receiving buffer anymore, free.
    
    err = interpret_response(req_code, indata);
    if (err != ERR_SUCCESS) {
        cerr << "ERROR INTERPRET: " << err << endl;
        return false;
    }
    
    return true;
}

bool Requests::get_response(time_t timeout) {
    string indata = "";
    RequestCodes req_code;
    ErrorCodes err;
    
    if (!_in_packet.receive_packet(_socket, timeout)) {
        cerr << "ERROR RECEIVE PACKET" << endl;
        close(_socket);
        return false;
    }
    
    if (!_in_packet.check_crc()) {
        mlog.log_error("crc error");
        close(_socket);
        return false;
    }
    
    req_code = (RequestCodes)_in_packet.get_request_code();
    indata = _in_packet.get_data();
    
    _in_packet.free_buffer(); // i wont use receiving buffer anymore, free.
    
    err = interpret_response(req_code, indata);
    if (err != ERR_SUCCESS) {
        cerr << "ERROR INTERPRET: " << err << endl;
        return false;
    }
    
    return true;
}

ErrorCodes Requests::check_request() {
    ErrorCodes ret = ERR_REQ_UNKNOWN;
    
    // check header
    if (_in_packet.get_header() != REQUEST_HEADER)
        return ERR_REQ_WRONG_HEADER;
    
    // check crc
    if (!_in_packet.check_crc())
        return ERR_REQ_CRC;
    
    return ERR_SUCCESS;
}

int g_question_no = 0;

ErrorCodes Requests::interpret_response(RequestCodes req_code, string indata) {
    
    mlog.log_debug("Request code: %d", req_code);
    
    if (req_code == REQ_FB_LOGIN) {
        // save jwt token
        g_user_info.token = indata;
        
        mlog.log_debug("token: %s", g_user_info.token.c_str());
        
        // parse jwt token for uid
        Jwt jtoken = Jwt(indata, jwt_key);
        
        // get and save uid
        g_user_info.uid = jtoken.get_uid();
        
        _next_request = REQ_MATCH;
    }
    else if (req_code == REQ_GET_ONLINE_USERS) {
        int count = (indata[0] << 0 & 0xFF) | ((indata[1] << 8) & 0xFF00);
        mlog.log_debug("count: %d", count);
        for (int i = 0; i < count; i++) {
            char *cpy = (char *)indata.c_str();
            mlog.log_hex("cpy", cpy, 10);
            uint64_t uid = GET_64(cpy + i*8 + 2);
            
            mlog.log_debug("id: %lu", uid);
        }
    }
    else if(req_code == REQ_MATCH) {
        nlohmann::json match_response_json;
        
        if (nlohmann::json::accept(indata)) {
            match_response_json = nlohmann::json::parse(indata);
        }
        else {
            mlog.log_debug("send match requests' ack received");
            return ERR_SUCCESS;
        }
        
        // print opponent data
        string username = match_response_json["name"];
        uint64_t op_uid = match_response_json["id"];
        int game_id = match_response_json["game_id"];
        mlog.log_debug("matched with %s [%d]", username.c_str(), op_uid);
        
        GameClient *game = GameClient::get_instance();
        
        game->set_uid(1, g_user_info.uid);
        game->set_uid(0, op_uid);
        game->set_game_id(game_id);
        
        _next_request = REQ_GAME_START;
    }
    else if (req_code == REQ_CANCEL_MATCH) {
        // receive ack
        mlog.log_debug("match cancelled");
        // TODO: something
        
    }
    else if (req_code == REQ_GAME_START) {
        mlog.log_debug("game start ack");
    }
    else if (req_code == REQ_GAME_ACCEPTED) {
        mlog.log_debug("game started");
        g_question_no = 0;
        GameClient *game = GameClient::get_instance();
        nlohmann::json questions = nlohmann::json::parse(indata);
        
        game->set_questions(questions);
        
        _next_request = REQ_GAME_ANSWER;
    }
    else if (req_code == REQ_GAME_ANSWER) {
        // receive ack
        mlog.log_debug("send answer's ack received");
    }
    else if (req_code == REQ_GAME_OPPONENT_ANSWER) {
        // receive opponent answer
        mlog.log_debug("opponent answer received");
        mlog.log_debug("%s", indata.c_str());
        
        nlohmann::json op_answer_json = nlohmann::json::parse(indata);
        string op_answer = op_answer_json["answer"];
        
        GameClient *game = GameClient::get_instance();
        if (game->check_answer(game->get_op_uid(), op_answer)) {
            mlog.log_info("opponent's answer is right");
        }
        else {
            mlog.log_info("opponent's answer is wrong");
        }
    }
    else if (req_code == REQ_GAME_QUESTION_COMPLETED) {
        // count questions
        g_question_no++;
        
        mlog.log_debug("next question %d\n", g_question_no);
        
        GameClient *game = GameClient::get_instance();
        if (game->finish_question()) {
            // if last question, send finish game request
            _next_request = REQ_GAME_FINISH;
        }
        else {
            _next_request = REQ_GAME_ANSWER;
        }
    }
    else if (req_code == REQ_GAME_OPPONENT_RESIGNED) {
        
        mlog.log_debug("opponent resigned\n");
        
        // send game finish request
        _next_request = REQ_GAME_FINISH;
    }
    else if (req_code == REQ_GAME_OPPONENT_TIMEOUT) {
        mlog.log_debug("opponent has timed out\n");
        
        // send game finish request
        _next_request = REQ_GAME_FINISH;
    }
    else if (req_code == REQ_ERROR) {
        // TODO: print error
        mlog.log_error("error\n");
    }
    
    return ERR_SUCCESS;
}

void Requests::prepare_error_packet(ErrorCodes err) {
    _out_packet.set_request_code(REQ_ERROR);
    string errcode = to_string(err);
    _out_packet.add_data(errcode);
    _out_packet.set_crc();
}

bool Requests::set_header(uint8_t header) {
    return _out_packet.set_header(header);
}

bool Requests::set_request_code(RequestCodes req_code) {
    return _out_packet.set_request_code((uint8_t)(req_code & 0xFF));
}

bool Requests::set_token(string token) {
    return _out_packet.add_data(token);
}

bool Requests::add_data(string data) {
    _out_packet.add_data(data);
    return true;
}

bool Requests::add_data(uint8_t *data, uint16_t len) {
    return _out_packet.add_data(data, len);
}

RequestCodes Requests::get_next_requets() {
    return _next_request;
}

void Requests::set_next_requets(RequestCodes req_code) {
    _next_request = req_code;
}

void Requests::clear_out_packet() {
    mlog.log_debug("clear out packet");
    _out_packet.clear();
}

void Requests::clear_in_packet() {
    _in_packet.clear();
}