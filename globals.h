#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <string>

typedef struct {
    std::string token = "";
    std::string fb_token = "";
    int current_game_id = 0;
    uint64_t uid = 0;
}UserInformation;

extern UserInformation g_user_info;

#endif // _GLOBALS_H_