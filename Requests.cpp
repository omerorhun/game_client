#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "Jwt.h"
#include "Protocol.h"
#include "utilities.h"
#include "json.hpp"

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

extern string g_token;

Requests::Requests(int sock) {
    socket = sock;
}

Requests::~Requests() {
    printf("request destructor\n");
}

void Requests::send_request(RequestCodes code, string data) {
    set_header(REQUEST_HEADER);
    set_request_code(code);
    if (code != REQ_FB_LOGIN)
        add_data(g_token);
    add_data(data);
    _out_packet.set_crc();
    
    _out_packet.send_packet(socket);
}

void Requests::send_response() {
    _out_packet.send_packet(socket);
}

bool Requests::get_response() {
    string indata = "";
    RequestCodes req_code;
    ErrorCodes err;
    
    if (!_in_packet.receive_packet(socket)) {
        cerr << "ERROR RECEIVE PACKET" << endl;
        return false;
    }
    
    if (!_in_packet.check_crc()) {
        cout << "crc error" << endl;
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

ErrorCodes Requests::interpret_response(RequestCodes req_code, string indata) {
    print_hex((const char *)"indata", (char *)indata.c_str(), indata.size());
    
    cout << "Request code: " << req_code << endl;
    
    _out_packet.set_header(REQUEST_HEADER);
    if (req_code == REQ_FB_LOGIN) {
        g_token = indata;
        printf("g_token: %s\n", g_token.c_str());
    }
    else if (req_code == REQ_GET_ONLINE_USERS) {
        int count = (indata[0] << 0 & 0xFF) | ((indata[1] << 8) & 0xFF00);
        printf("count: %d\n", count);
        for (int i = 0; i < count; i++) {
            int id = 
                ((indata[i*4 + 2] << 0) & 0xFF) |
                ((indata[i*4 + 3] << 8) & 0xFF00) |
                ((indata[i*4 + 4] << 16) & 0xFF0000) |
                ((indata[i*4 + 5] << 24) & 0xFF000000);
            printf("id: %d\n", id);
        }
    }
    else if(req_code == REQ_MATCH) {
        nlohmann::json user_json;
        
        if (nlohmann::json::accept(indata)) {
            user_json = nlohmann::json::parse(indata);
        }
        else {
            printf("not json data\n");
            return ERR_REQ_UNKNOWN;
        }
        
        // print opponent data
        int op_uid = 
                ((indata[0] << 0) & 0xFF) |
                ((indata[1] << 8) & 0xFF00) |
                ((indata[2] << 16) & 0xFF0000) |
                ((indata[3] << 24) & 0xFF000000);
        
        string username = user_json["name"];
        op_uid = user_json["id"];
        
        printf("matched with %s [%d]\n", username.c_str(), op_uid);
    }
    else if (req_code == REQ_CANCEL_MATCH) {
        printf("match cancelled\n");
    }
    else if (req_code == REQ_START_GAME) {
        printf("game started\n");
        printf("indata: %s\n", indata.c_str());
    }
    else if (req_code == REQ_ERROR) {
        // TODO: print error
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