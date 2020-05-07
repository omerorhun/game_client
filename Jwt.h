#ifndef _JWT_H_
#define _JWT_H_

#include <string>
#include <stdint.h>

#define TOKEN_LIFETIME_SEC 1800 // 30 min

class Jwt {
    public:
    Jwt(std::string token, std::string key);
    Jwt(uint64_t user_id, long expire, std::string key);
    std::string get_token();
    bool verify();
    uint64_t get_uid();
    
    private:
    std::string _header;
    std::string _payload;
    std::string _signature;
    std::string _key;
    std::string _token;
    bool _is_valid;
    
    std::string sign_packet();
    std::string generate_token();
    bool encode();
    bool decode(); // parse token
    
    bool is_expired();
};

#endif // _JWT_H_