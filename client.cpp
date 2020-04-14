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
#include "debug.h"

#define CONTINOUS_CONN 1

#define HOST_ADDR "0.0.0.0"
//#define HOST_ADDR "134.122.89.177"
#define HOST_PORT 1903
#define BUFFER_SIZE 4096
#define HOST_ADDR_SIZE 64
using namespace std;

void *listener(void *arg);
void *listen_response(void *arg);
void print_usage();
void print_client_status(sockaddr_in client);
bool get_args(int argc, char **argv, char *hostaddr, char **ipaddr, uint16_t *port);
void start_manuel_mode(int sockfd);
void start_bot(int sockfd);
FILE *generate_filename();

string g_token = "";

string g_fbToken = "";
Dlogger mlog;

int main (int argc, char **argv) {
    //int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char host_addr[HOST_ADDR_SIZE];
    char *ip_addr;
    uint16_t port = HOST_PORT;
    
    FILE *fd = generate_filename();
    
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
    
    if (g_fbToken.empty()) {
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
    RequestCodes req = REQ_FB_LOGIN;
    
    while (1) {
        if (req == REQ_FB_LOGIN) {
            mlog.log_debug("sending facebook login request");
            Requests request(sockfd);
            request.send_request(REQ_FB_LOGIN, g_fbToken);
            request.get_response(0);
            req = request.get_next_requets();
            req = REQ_MATCH;
        }
        else if (req == REQ_MATCH) {
            mlog.log_debug("sending match request");
            Requests request(sockfd);
            request.send_request(REQ_MATCH, "");
            request.get_response(0);
            req = REQ_START_GAME;
        }
        else if (req == REQ_START_GAME) {
            mlog.log_debug("sending start game request");
            Requests request(sockfd);
            request.send_request(REQ_START_GAME, "");
            request.get_response(0);
            break;
        }
    }
    
    close(sockfd);
}

void start_manuel_mode(int sockfd) {
    
    while (1) {
        int input = 2;
        mlog.log_debug("enter the command: ");
        while (scanf("%d", &input) != 1);

        if (input == 1 || input == 5) {
            // send "fb login" request
            mlog.log_debug("sending login with facebook request...");
            
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAEfRmgpICrcdZCOvOrZAT5NqPmEIdO9KTknvk3uIgFJVDLR18qqXzx5JxbwZB0uxlBXABzfYAUD9j7eHoYgKI93Bebd9ZCNNWhzWD0cKxg3PJZCHd6xNvfMHW2HzdffYgaOliTpnQL5axWmIQUBtlxXE6syibj0xspfPfIBxqjUgBEjeqkIPahgR7ztixo3Ce4lcZAGzoC&data_access_expiration_time=1594639957&expires_in=5243
	        //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAKGodLSwSSqLw6sjGQRToCZCzdp7HeZAU3mohyAnd7vJdjXZCCuyeb3I3TYpKWbLCxGimh8v3ZCESfJCsH1siZBvtDnZAvXJylArZAaRzKcEZBpqbRd5lIkJMkeIMZBZCQPZBZAbrbeM5Ui2CbmubvOdTUPgu2O35IEuhZCdR8d3ZCXiLe8D7N3gDjppIZD&data_access_expiration_time=1594447770&expires_in=6630
            
            // ali veli's access token
            string ali_token =
	    "EAAJQZBZANTOG0BAEfRmgpICrcdZCOvOrZAT5NqPmEIdO9KTknvk3uIgFJVDLR18qqXzx5JxbwZB0uxlBXABzfYAUD9j7eHoYgKI93Bebd9ZCNNWhzWD0cKxg3PJZCHd6xNvfMHW2HzdffYgaOliTpnQL5axWmIQUBtlxXE6syibj0xspfPfIBxqjUgBEjeqkIPahgR7ztixo3Ce4lcZAGzoC";
            // ömer's access token
            
            string omer_token = "EAAJQZBZANTOG0BAOAjRFp7jriJWv0ThQgcxxBxEBTl8KrmXDc0UUTTSzjhE85hsCGYSlu7J7B8vOpLJvTSS5xploUzfkFaLh8cNMZBBnG3TFmyJsHUckJeCGAZBo3wOBoZAbvWxdFlaGnxYQjO74aNZAbJwbOO1C8ZAh9vuZCOT6RkwUVt4NmwfnIyGvzurKqVwZD";
            string access_token;
            if (input == 5)
                access_token = omer_token;
            else
                access_token = ali_token;
            
            Requests request(sockfd);
            request.send_request(REQ_FB_LOGIN, access_token);
        }
        else if (input == 2) {
            // send "get online users" request
            mlog.log_debug("sending get online users request...");
            
            Requests request(sockfd);
            request.send_request(REQ_GET_ONLINE_USERS, "");
        }
        else if (input == 3) {
            // send logout request
            mlog.log_debug("sending logout request...");
            
            Requests request(sockfd);
            request.send_request(REQ_LOGOUT, "");
        }
        else if (input == 4) {
            // send match request
            mlog.log_debug("sending match request...");
            
            Requests request(sockfd);
            request.send_request(REQ_MATCH, "");
        }
        else if (input == 6) {
            // accept game
            
            Requests request(sockfd);
            request.send_request(REQ_START_GAME, "");
        }
        else if (input == 7) {
            // reject game
            Requests request(sockfd);
            request.send_request(REQ_CANCEL_MATCH, "");
        }
        else {
            close(sockfd);
            return;
        }
        
        Requests response(sockfd);
        response.get_response(20);
    }
    
}

void *listen_response(void *arg) {
    int sockfd = *((int *)arg);
    Requests response(sockfd);
    
    while (1) {
        if (!response.get_response(0))
            break;
    }
    
    close(sockfd);
    return NULL;
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
            g_fbToken = string(ptr);
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
    mlog.log_info("3: send 'logout' request\n");
}

FILE *generate_filename() {
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
        
        sprintf(filename, "%s/client_%02d-%02d-%02d.log", filepath,
                                                        tm_st->tm_hour,
                                                        tm_st->tm_min,
                                                        tm_st->tm_sec);
        if (try_count > 0) {
            int len = strlen(filename);
            sprintf(&filename[len], "-%d", try_count);
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
