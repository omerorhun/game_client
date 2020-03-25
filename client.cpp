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

#include <pthread.h>

#include "Requests.h"

#define HOST_ADDR "0.0.0.0"
//#define HOST_ADDR "134.122.89.177"
#define HOST_PORT 1903
#define BUFFER_SIZE 4096

using namespace std;

void *listener(void *arg);
void *listen_response(void *arg);
void print_usage();

int main () {
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, HOST_ADDR, &server_addr.sin_addr);
    
    int ret = connect(sockfd, (const sockaddr *)&server_addr, sizeof(server_addr));
    
    if (ret == -1) {
        cerr << "Can't connect to server\n";
        return -1;
    }
    
    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, listen_response, (void *)&sockfd);
    
    print_usage();
    
    while (1) {
        int input;
        while (scanf(" %d", &input) != 1);
        
        if (input == 1) {
            // send "fb login" request
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAMlKrYd5EWok0NtT4pgaAoZCvg6TsEjDiptJE2ZCEdAJ3sPdi6WEbFAl19cYAKWNioeWAD9Cj5Y9b3LN0AnN0KMQcpNKlDsFZAHh1CKCcn42pEoccVfqhK6hgi77wLRYSJYNZAjCFkmTCNXWewgtFiQwJu8UQgl01wZBPsHQLhfMOy6PGZAjg8G4DOScZCs31LRCYEHenIP&data_access_expiration_time=1592853060&expires_in=6539
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAPueEjX2jgNYqc3br7agj0rRYOmWMtyLsbG8N0Xous0S0VfYcikloxLBVZClD0ZCZBlOvWz5TbG3n4LRp9L8wWRWMHrSlaw0r4dGwDOX5bCqDwN3VgVkrPmUQlKQSNTH1b1XrDNqC2No7C8cRafItvxRKxOVAai6ZBBTpSsUF3exoyVn8cDJa65fIFYpjRd7QZBNaK2KH&data_access_expiration_time=1592873759&expires_in=3840
            string access_token = "EAAJQZBZANTOG0BAPueEjX2jgNYqc3br7agj0rRYOmWMtyLsbG8N0Xous0S0VfYcikloxLBVZClD0ZCZBlOvWz5TbG3n4LRp9L8wWRWMHrSlaw0r4dGwDOX5bCqDwN3VgVkrPmUQlKQSNTH1b1XrDNqC2No7C8cRafItvxRKxOVAai6ZBBTpSsUF3exoyVn8cDJa65fIFYpjRd7QZBNaK2KH";
            Requests request(sockfd);
            request.send_request(REQ_FB_LOGIN, access_token);
        }
        else if (input == 2) {
            // send "get online users" request
            Requests request(sockfd);
            request.send_request(REQ_GET_ONLINE_USERS, "");
        }
    }
    
    close(sockfd);
    
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

void print_usage() {
    cout << "1: send 'fb login' request" << endl;
    cout << "2: send 'get online clients' request" << endl << endl;
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


