#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <pthread.h>

#include "Requests.h"
#include "GameClient.h"
#include "debug.h"
#include "json.hpp"
#include "globals.h"

#include <queue>
#include <mutex>

#define CONTINOUS_CONN 1
#define CLIENT_ANSWER_DELAY_US (2000*1000)
#define HOST_ADDR "0.0.0.0"
#define HOST_PORT 1903
#define BUFFER_SIZE 4096
#define HOST_ADDR_SIZE 64


using namespace std;



void *listener(void *arg);
void *listen_for_notifications(void *arg);
void *recv_response(void *arg);
void print_usage();
void print_client_status(sockaddr_in client);
bool get_args(int argc, char **argv, char *hostaddr, char **ipaddr, uint16_t *port);
void start_manuel_mode(int sockfd);
void start_bot(int sockfd);
FILE *generate_log_file();

queue<string> responses;
mutex g_responses_mtx;
Dlogger mlog;
bool gb_is_active = false;
UserInformation g_user_info;

int main (int argc, char **argv) {
    srand(time(NULL));
    g_user_info.current_game_id = 0;
    g_user_info.token = "";
    //int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char host_addr[HOST_ADDR_SIZE];
    char *ip_addr;
    uint16_t port = HOST_PORT;
    
    FILE *fd = generate_log_file();
    
    if (fd == NULL)
        return 0;
    
    mlog.log_reset_fp();
    mlog.log_set_fp(fd);
    
    mlog.log_info("new client started");
    
    if(!get_args(argc, argv, host_addr, &ip_addr, &port)) {
        mlog.log_debug("usage: client host=<host-address> port=<port> token=<token>");
        return 0;
    }
    
    mlog.log_debug("host: %s", host_addr);
    mlog.log_debug("port: %hu", port);
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, (const char *)ip_addr, &server_addr.sin_addr.s_addr);
    
    print_client_status((sockaddr_in)server_addr);
    print_usage();
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(sockfd, (const sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        mlog.log_error("Can't connect to server");
        return -1;
    }
    
    if (g_user_info.fb_token.empty()) {
        mlog.log_info("manuel mode");
        start_manuel_mode(sockfd);
    }
    else {
        mlog.log_info("bot mode");
        start_bot(sockfd);
    }
    
    return 0;
}

void start_bot(int sockfd) {
    pthread_t listen_thread, response_thread;
    Requests request(sockfd);
    
    pthread_create(&listen_thread, NULL, listen_for_notifications, (void *)&sockfd);
    pthread_create(&response_thread, NULL, recv_response, (void *)&request);
    
    request.set_next_requets(REQ_FB_LOGIN);
    
    gb_is_active = true;
    
    while (1) {
        RequestCodes req = request.get_next_requets();
        if (req != REQ_IDLE)
            request.clear_out_packet();
        
        if (req == REQ_FB_LOGIN) {
            mlog.log_debug("sending facebook login request");
            request.send_request(REQ_FB_LOGIN, g_user_info.fb_token);
            request.set_next_requets(REQ_IDLE);
        }
        else if (req == REQ_MATCH) {
            mlog.log_debug("sending match request");
            request.send_request(REQ_MATCH, "");
            request.set_next_requets(REQ_IDLE);
        }
        else if (req == REQ_GAME_START) {
            mlog.log_debug("sending start game request");
            
            nlohmann::json json_data;
            json_data["game_id"] = GameClient::get_instance()->get_game_id();
            json_data["category"] = (uint8_t)1;
            
            
            useconds_t answer_delay = CLIENT_ANSWER_DELAY_US;
            usleep(answer_delay);
            request.send_request(REQ_GAME_START, json_data.dump());
            request.set_next_requets(REQ_IDLE);
        }
        else if (req == REQ_GAME_ANSWER) {
            mlog.log_debug("sending game answer request");
            GameClient *game = GameClient::get_instance();
            
            nlohmann::json json_data;
            
            char answer[2]  = {0,0};
            answer[0] = 'a' + (char)(rand()%4);
            json_data["answer"] = answer;
            json_data["game_id"] = game->get_game_id();
            
            if (game->check_answer(game->get_my_uid(), string(answer))) {
                mlog.log_info("your answer is right");
            }
            else {
                mlog.log_info("your answer is wrong");
            }
            
            useconds_t answer_delay = CLIENT_ANSWER_DELAY_US;
            usleep(answer_delay);
            request.send_request(REQ_GAME_ANSWER, json_data.dump());
            request.set_next_requets(REQ_IDLE);
        }
        else if (req == REQ_GAME_FINISH) {
            mlog.log_debug("sending game finish request");
            
            GameClient *game = GameClient::get_instance();
            uint8_t category = 1;
            
            nlohmann::json json_data;
            json_data["game_id"] = game->get_game_id();
            nlohmann::json results;
            results["winner"] = game->get_winner();
            
            nlohmann::json users;
            for (int i = 0; i < 2; i++) {
                GameUser me = game->get_results(i);
                
                nlohmann::json user;
                user["uid"] = me.uid;
                for (int j = 0; j < 3; j++) {
                    nlohmann::json tour;
                    tour["category"] = me.tour_result[j].category;
                    tour["right"] = me.tour_result[j].right;
                    tour["wrong"] = me.tour_result[j].wrong;
                    
                    user["tours"].push_back(tour);
                }
                users.push_back(user);
            }
            
            results["users"] = users;
            json_data["results"] = results;
            
            useconds_t answer_delay = CLIENT_ANSWER_DELAY_US;
            usleep(answer_delay);
            request.send_request(REQ_GAME_FINISH, json_data.dump());
            break;
        }
        else if (req == REQ_IDLE) {
            // do nothing
        }
        
        if (!gb_is_active)
            break;
    }
    
    gb_is_active = false;
    
    close(sockfd);
}

void start_manuel_mode(int sockfd) {
    pthread_t listen_thread, responses_thread;
    Requests request(sockfd);
    
    pthread_create(&listen_thread, NULL, listen_for_notifications, (void *)&sockfd);
    pthread_create(&responses_thread, NULL, recv_response, (void *)&request);
    
    gb_is_active = true;
    
    while (1) {
        int input = 2;
        mlog.log_debug("enter the command: ");
        
        while (scanf("%d", &input) != 1);
        
        request.clear_out_packet();
        
        if (input == 1 || input == 5) {
            // send "fb login" request
            mlog.log_debug("sending login with facebook request...");
            
	        //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAHFanKkdeWByg5UweRwvFM32sS4ZCXpbCtvh4Un8mGvi2C6rS88ZCeM3ZC8uQHvMyFP1y4g5tlXzHvE2dHPXttULcaCWeCOqLIk38U1DfvyLOJtWaGI6iRRWj17H0Bt4JhndQQwcqS7knBsCrvf8EXJgCaZBjfJB0R5VDTOrlciMj5Br68hGvjTfAoXsaKZCHu0hS21rc&data_access_expiration_time=1596690182&expires_in=7018
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BALDbLhxI0U2NYnCyxjE30few0bR9P5Vbq0CEieUA0hzMaATgAWRLPUbixVy86cEnjziLFO8kaGALJpOkrIpy3pAbGFq4weZAOn9JgpgDy6YA7bMAAsW1ISPkXVRA6WTwvu4ejvQtoSYQwSluaFWSKTFWcuC0EUfZCD92wFW72NyATzGFMZD&data_access_expiration_time=1596690200&expires_in=7000
            
            // ali veli's access token
            string ali_token =
	    "EAAJQZBZANTOG0BAHFanKkdeWByg5UweRwvFM32sS4ZCXpbCtvh4Un8mGvi2C6rS88ZCeM3ZC8uQHvMyFP1y4g5tlXzHvE2dHPXttULcaCWeCOqLIk38U1DfvyLOJtWaGI6iRRWj17H0Bt4JhndQQwcqS7knBsCrvf8EXJgCaZBjfJB0R5VDTOrlciMj5Br68hGvjTfAoXsaKZCHu0hS21rc";
            // ömer's access token
            string omer_token = "EAAJQZBZANTOG0BALDbLhxI0U2NYnCyxjE30few0bR9P5Vbq0CEieUA0hzMaATgAWRLPUbixVy86cEnjziLFO8kaGALJpOkrIpy3pAbGFq4weZAOn9JgpgDy6YA7bMAAsW1ISPkXVRA6WTwvu4ejvQtoSYQwSluaFWSKTFWcuC0EUfZCD92wFW72NyATzGFMZD";
            string access_token;
            if (input == 5)
                access_token = omer_token;
            else
                access_token = ali_token;
            
            request.send_request(REQ_FB_LOGIN, access_token);
        }
        else if (input == 2) {
            // send "get online users" request
            mlog.log_debug("sending get online users request...");
            request.send_request(REQ_GET_ONLINE_USERS, "");
        }
        else if (input == 3) {
            // send logout request
            mlog.log_debug("sending logout request...");
            request.send_request(REQ_LOGOUT, "");
        }
        else if (input == 4) {
            // send match request
            mlog.log_debug("sending match request...");
            
            request.send_request(REQ_MATCH, "");
        }
        else if (input == 6) {
            // accept game
            mlog.log_debug("sending game start request...");
            
            nlohmann::json json_data;
            json_data["game_id"] = GameClient::get_instance()->get_game_id();
            json_data["category"] = (uint8_t)1;
            
            request.send_request(REQ_GAME_START, json_data.dump());
        }
        else if (input == 7) {
            // reject game
            mlog.log_debug("sending cancel match request...");
            
            nlohmann::json json_data;
            json_data["game_id"] = GameClient::get_instance()->get_game_id();
            
            request.send_request(REQ_CANCEL_MATCH, json_data.dump());
        }
        else if (input == 8) {
            // send game answer request
            mlog.log_debug("sending game answer request...");
            GameClient *game = GameClient::get_instance();
            nlohmann::json json_data;
            
            char answer[2]  = {0,0};
            answer[0] = 'a' + (char)(rand()%4);
            json_data["answer"] = answer;
            json_data["game_id"] = game->get_game_id();
            
            if (game->check_answer(game->get_my_uid(), string(answer))) {
                mlog.log_info("your answer is right");
            }
            else {
                mlog.log_info("your answer is wrong");
            }
            
            request.send_request(REQ_GAME_ANSWER, json_data.dump());
        }
        else if (input == 9) {
            // send resign request
            mlog.log_debug("sending game resign request...");
            
            nlohmann::json json_data;
            json_data["game_id"] = GameClient::get_instance()->get_game_id();
            
            request.send_request(REQ_GAME_RESIGN, json_data.dump());
        }
        else if (input == 10) {
            // send game finish request
            mlog.log_debug("sending game finish request...");
            GameClient *game = GameClient::get_instance();
            uint8_t category = 1;
            
            nlohmann::json json_data;
            json_data["game_id"] = game->get_game_id();
            nlohmann::json results;
            results["winner"] = game->get_winner();
            
            nlohmann::json users;
            for (int i = 0; i < 2; i++) {
                GameUser me = game->get_results(i);
                
                nlohmann::json user;
                user["uid"] = me.uid;
                for (int j = 0; j < 3; j++) {
                    nlohmann::json tour;
                    tour["category"] = me.tour_result[j].category;
                    tour["right"] = me.tour_result[j].right;
                    tour["wrong"] = me.tour_result[j].wrong;
                    
                    user["tours"].push_back(tour);
                }
                users.push_back(user);
            }
            
            results["users"] = users;
            json_data["results"] = results;
            
            request.send_request(REQ_GAME_FINISH, json_data.dump());
        }
        else {
            gb_is_active = false;
            close(sockfd);
            return;
        }
        
        if (!gb_is_active)
            break;
    }
    
}

void *recv_response(void *arg) {
    Requests *response = (Requests*)arg;
    
    while(1) {
        if (responses.size()) {
            string next_response = responses.front();
            
            g_responses_mtx.lock();
            responses.pop();
            g_responses_mtx.unlock();
            
            if (!response->get_response(next_response))
                break;
        }
    }
    
    gb_is_active = false;
}

void *listen_for_notifications(void *arg) {
    int sockfd = *((int *)arg);
    char rx_buffer[RX_BUFFER_SIZE];
    uint16_t rx_len = 0;
    uint16_t expected = RX_BUFFER_SIZE; // set expected size to max
    bool ignore = true;
    
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
    
    gb_is_active = true;
    
    mlog.log_debug("listening for notifications...");
    while (1) {
        if (!gb_is_active)
            break;
        
        char ch = 0;
        if (recv(sockfd, &ch, 1, 0) == -1) {
            gb_is_active = false;
            break;
        }
        
        // connection closed by server
        if ((ch == 0) && (rx_len == 0)) {
            mlog.log_debug("connection closed by peer");
            gb_is_active = false;
            break;
        }
        
        if (ch == REQUEST_HEADER) {
            // start to store data
            mlog.log_debug("header received");
            ignore = false;
        }
        
        // if data is useless, ignore
        if (ignore) {
            mlog.log_warn("ignore: 0x%02x ", ch);
            continue;
        }
        
        // add byte to the buffer
        rx_buffer[rx_len++] = ch;
        
        // if 3 bytes received, then get the length of the request
        if (rx_len == 3) {
            expected = GET_LENGTH(rx_buffer) + 5;
        }
        
        if (rx_len == expected) {
            ignore = true;
            
            // transmit the request to the interpreter
            g_responses_mtx.lock();
            responses.push(string((char *)rx_buffer, rx_len));
            g_responses_mtx.unlock();
            
            memset(rx_buffer, 0, RX_BUFFER_SIZE);
            rx_len = 0;
            
            // set expected size to max, until a real packet received
            expected = RX_BUFFER_SIZE;
        }
    }
}

bool get_args(int argc, char **argv, char *hostaddr, char **ipaddr, uint16_t *port) {
    memset(hostaddr, 0, HOST_ADDR_SIZE);
    
    // set default values
    strcpy(hostaddr, HOST_ADDR);
    *port = HOST_PORT;
    
    
    // check token, host, port, userid
    for (int i = 0; i < argc; i++) {
        
        char *ptr = strstr(argv[i], "=");
        ptr++;
        
        if (strstr(argv[i], "port=")) {
            // get port no
            uint16_t temp = atoi(ptr);
            if (isdigit(ptr[0])) {
                *port = temp;
            }
        }
        else if (strstr(argv[i], "host=")) {
            // get host name
            strcpy(hostaddr, ptr);
        }
        else if (strstr(argv[i], "token=")) {
            // get token value
            g_user_info.fb_token = string(ptr);
        }
        else if (strstr(argv[i], "userid=")) {
            // do nothing for now
        }
        else {
            // do nothing
        }
    }
    
    hostent *addr = gethostbyname(hostaddr);
    if (addr == NULL) {
        mlog.log_error("Error: Invalid host-address");
        return false;
    }
    
    *ipaddr = inet_ntoa(*((struct in_addr*)addr->h_addr_list[0]));
    
    return true;
}

void print_usage() {
    mlog.log_info("1: send 'fb login' request");
    mlog.log_info("2: send 'get online clients' request");
    mlog.log_info("3: send 'logout' request");
    mlog.log_info("4: send 'match' request");
    mlog.log_info("5: send 'login as ömer' request");
    mlog.log_info("6: send 'accept game' request");
    mlog.log_info("7: send 'reject game' request");
    mlog.log_info("8: send 'game answer' request");
    mlog.log_info("9: send 'resign' request");
    mlog.log_info("10: send 'game finish' request");
}

FILE *generate_log_file() {
    char command[64];
    char filepath[32];
    char filename[32];
    time_t now = time(NULL);
    struct tm *tm_st = localtime(&now);
    
    memset(command, 0, 64);
    memset(filepath, 0, 32);
    sprintf(filepath, "logs/%04d-%02d-%02d", tm_st->tm_year + 1900,
                                                        tm_st->tm_mon + 1,
                                                        tm_st->tm_mday);
    sprintf(command, "mkdir -p %s", filepath);
    
    // create path if doesn't exists
    system(command);
    int try_count = 0;
    do {
        memset(filename, 0, 64);
        
        sprintf(filename, "%s/client_%02d-%02d-%02d", filepath,
                                                        tm_st->tm_hour,
                                                        tm_st->tm_min,
                                                        tm_st->tm_sec);
        if (try_count > 0) {
            int len = strlen(filename);
            sprintf(&filename[len], "-%d.log", try_count);
        }
        else {
            int len = strlen(filename);
            sprintf(&filename[len], ".log");
        }
        
        try_count++;
    } while (access(filename, F_OK) != -1);
    
    return fopen(filename, "a+");
}

void print_client_status(sockaddr_in client) {
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    
    memset(&host, 0, NI_MAXHOST);
    memset(&svc, 0, NI_MAXSERV);
    
    int result = getnameinfo((const sockaddr *)&client, sizeof(client), 
                                            host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    
    if (result) {
        mlog.log_info("%s connected to %s" , host, svc);
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        mlog.log_info("%s connected to %hu", host, ntohs(client.sin_port));
    }
    
    return;
}
