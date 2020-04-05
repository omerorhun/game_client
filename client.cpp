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
string g_token = "";

int main (int argc, char **argv) {
    //int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char host_addr[HOST_ADDR_SIZE];
    char *ip_addr;
    uint16_t port = HOST_PORT;
    
    if(!get_args(argc, argv, host_addr, &ip_addr, &port)) {
        printf("usage: client <host-address> <*port>\n");
        printf("*: optional\n");
        return 0;
    }
    
    printf("host: %s\n", host_addr);
    printf("port: %hu\n", port);
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, (const char *)ip_addr, &server_addr.sin_addr.s_addr);
    
    print_client_status((sockaddr_in)server_addr);
    print_usage();

#if CONTINOUS_CONN
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(sockfd, (const sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        cerr << "Can't connect to server\n";
        return -1;
    }
    
    while (1) {
        int input = 2;
        printf("enter the command: ");
        while (scanf("%d", &input) != 1);
#else
    while (1) {
        int input;
        printf("enter the command: ");
        while (scanf("%d", &input) != 1);
        
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        int ret = connect(sockfd, (const sockaddr *)&server_addr, sizeof(server_addr));
        if (ret == -1) {
            cerr << "Can't connect to server\n";
            return -1;
        }
#endif
        if (input == 1 || input == 5) {
            // send "fb login" request
            printf("sending login with facebook request...\n");
            
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAF9D9lCDUVVk6g1v5Rmh3Fzb74q1X0x1uArERn3ASwhdFr7n0c0sv3CYv33zrLYYq6As96TzS9OrzR26d710IDJaftc4HzZBvPrIgStYqJ3hpaoIvNRkqDV5leK3qvZBTDTYDwuZA0j8ZCbT797CBk3ydUMoL6ZAvk8HT5wLyUJ4AT9RUa09MyisHcZACKRszZBHtyr05oj&data_access_expiration_time=1593829431&expires_in=5769
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAPqnmzK8qGycRtkFPWbIuuTUoGXk0j8IhO0uchuakJefodEmlgy6RmR2d5PbifLCBAWW1p7iNFuMKyWgBnBgEZAKUJJ1Sg5knzeayGnv6ZALXQuahfZC3ZA2WIxg3GrQCK82daLia9Gs2ObLElgHXQE8fjW8qjzzm6dJPjeV9MKKZA12TBpoZD&data_access_expiration_time=1593829541&expires_in=5658
            // ali veli's access token
            string ali_token = "EAAJQZBZANTOG0BAF9D9lCDUVVk6g1v5Rmh3Fzb74q1X0x1uArERn3ASwhdFr7n0c0sv3CYv33zrLYYq6As96TzS9OrzR26d710IDJaftc4HzZBvPrIgStYqJ3hpaoIvNRkqDV5leK3qvZBTDTYDwuZA0j8ZCbT797CBk3ydUMoL6ZAvk8HT5wLyUJ4AT9RUa09MyisHcZACKRszZBHtyr05oj";
            // ömer's access token
            
            string omer_token = "EAAJQZBZANTOG0BAPqnmzK8qGycRtkFPWbIuuTUoGXk0j8IhO0uchuakJefodEmlgy6RmR2d5PbifLCBAWW1p7iNFuMKyWgBnBgEZAKUJJ1Sg5knzeayGnv6ZALXQuahfZC3ZA2WIxg3GrQCK82daLia9Gs2ObLElgHXQE8fjW8qjzzm6dJPjeV9MKKZA12TBpoZD";
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
            printf("sending get online users request...\n");
            
            Requests request(sockfd);
            request.send_request(REQ_GET_ONLINE_USERS, "");
        }
        else if (input == 3) {
            // send logout request
            printf("sending logout request...\n");
            
            Requests request(sockfd);
            request.send_request(REQ_LOGOUT, "");
        }
        else if (input == 4) {
            // send match request
            printf("sending match request...\n");
            
            Requests request(sockfd);
            request.send_request(REQ_MATCH, "");
        }
        else {
            close(sockfd);
            return 0;
        }
        
        Requests response(sockfd);
        response.get_response();
        
        //sleep(3);
#if CONTINOUS_CONN == 0
        close(sockfd);
#endif
    }
    
    return 0;
}

void *listen_response(void *arg) {
    int sockfd = *((int *)arg);
    Requests response(sockfd);
    
    while (1) {
        if (!response.get_response())
            break;
    }
    
    close(sockfd);
    return NULL;
}

bool get_args(int argc, char **argv, char *hostaddr, char **ipaddr, uint16_t *port) {
    if (argc > 3)
        return false;
    
    memset(hostaddr, 0, HOST_ADDR_SIZE);
    if (argc > 1) strcpy(hostaddr, argv[1]);
    else strcpy(hostaddr, HOST_ADDR);
    
    hostent *addr = gethostbyname(hostaddr);
    if (addr == NULL) {
        printf("Error: Invalid host-address\n");
        return false;
    }
    
    *ipaddr = inet_ntoa(*((struct in_addr*)addr->h_addr_list[0]));
    
    if (argc == 3) {
        uint16_t temp = atoi(argv[2]);
        
        if (isdigit(argv[2][0]))
            *port = temp;
    }
    
    return true;
}

void print_usage() {
    cout << "1: send 'fb login' request" << endl;
    cout << "2: send 'get online clients' request" << endl;
    cout << "3: send 'logout' request" << endl;
    cout << endl;
}

void print_client_status(sockaddr_in client) {
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    
    memset(&host, 0, NI_MAXHOST);
    memset(&svc, 0, NI_MAXSERV);
    
    int result = getnameinfo((const sockaddr *)&client, sizeof(client), 
                                            host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    
    if (result) {
        cout << string(host) << " connected to " << string(svc) << endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << string(host) << " connected to " << ntohs(client.sin_port) << endl;
    }
    
    return;
}

#if 0
void *listener(void *arg) {
    int sockfd = *((int *)arg);
    char rx_buffer[BUFFER_SIZE];
    
    while (1) {
        memset(&rx_buffer, 0, BUFFER_SIZE);
        
        int rx_size = recv(sockfd, rx_buffer, BUFFER_SIZE, 0);
        
        if (rx_size == 0)
            break;
        
        cout << "Server: " << string(rx_buffer) << endl;
        
    }
    
    close(sockfd);
    return NULL;
}
#endif


