#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "Jwt.h"
#include "Protocol.h"

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

Requests::Requests(int sock) {
    socket = sock;
}

void Requests::send_request(RequestCodes code, string data) {
    _out_packet.set_header(REQUEST_HEADER);
    _out_packet.set_request_code(code);
    _out_packet.add_data(data);
    _out_packet.set_crc();
    
    _out_packet.send_packet(socket);
}

void Requests::send_response() {
    _out_packet.send_packet(socket);
}

bool Requests::get_response() {
    if (!_in_packet.receive_packet(socket)) {
        cerr << "ERROR RECEIVE PACKET" << endl;
        return false;
    }
    
    if (!_in_packet.check_crc())
        cout << "crc error" << endl;
    
    _in_packet.free_buffer();
    return true;
}

RequestErrorCodes Requests::check_request() {
    RequestErrorCodes ret = ERR_REQ_UNKNOWN;
    
    // check header
    if (_in_packet.get_header() != REQUEST_HEADER)
        return ERR_REQ_WRONG_HEADER;
    
    // check crc
    if (!_in_packet.check_crc())
        return ERR_REQ_CRC;
    
    return ERR_REQ_SUCCESS;
}
#if 0
bool Requests::check_request_code() {
    if (_in_packet.get_request_code() < REQ_COUNT)
        return true;
    
    return false;
}
#endif
void Requests::handle_request() {
    RequestCodes req_code = (RequestCodes)_in_packet.get_request_code();
    
    cout << "Request code: " << req_code << endl;
    
    _out_packet.set_header(REQUEST_HEADER);
    if (req_code == REQ_FB_LOGIN) {
        
    }
    else if (req_code == REQ_GET_ONLINE_USERS) {
        
    }
    else {
        // unknown request received
        _in_packet.free_buffer();
        prepare_error_packet(ERR_REQ_WRONG_REQ_CODE);
    }
    
    return;
}

void Requests::prepare_error_packet(RequestErrorCodes err) {
    _out_packet.set_request_code(REQ_ERROR);
    string errcode = to_string(err);
    _out_packet.add_data(errcode);
    _out_packet.set_crc();
}
