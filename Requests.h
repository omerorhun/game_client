#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "Protocol.h"
#include "Jwt.h"

#define FB_TOKEN_SIZE 256
#define REQUEST_HEADER 0x01

const std::string jwt_key = "bjk1903";

typedef enum{
    ERR_REQ_SUCCESS = 0,
    ERR_REQ_WRONG_HEADER,
    ERR_REQ_WRONG_REQ_CODE,
    ERR_REQ_WRONG_LENGTH,
    ERR_REQ_CRC,
    ERR_REQ_UNKNOWN
}RequestErrorCodes;

typedef enum {
    REQ_LOGIN,
    REQ_FB_LOGIN,
    REQ_COUNT,
    REQ_LOGOUT,
    REQ_MATCH,
    REQ_GET_ONLINE_USERS,
    REQ_DISCONNECT,
    REQ_ERROR,
}RequestCodes;

static const int lengths[][2] = 
{
    {REQ_LOGIN, 0}, 
    {REQ_GET_ONLINE_USERS, 0},
};

#define GET_ONLINE_USERS_LEN 0

class Requests {
  
  public:

  Requests(int sock);
  ~Requests();
  
  bool get_response();
  RequestErrorCodes check_request();
  void handle_request();
  void send_response();
  void send_request(RequestCodes code, std::string data);
  
  private:
  Protocol _in_packet;
  Protocol _out_packet;
  
  int socket;
  
  RequestCodes req_code;
  
  bool check_request_code();
  bool check_length();
  void prepare_error_packet(RequestErrorCodes err);
};



#endif /* _REQUESTS_H_*/