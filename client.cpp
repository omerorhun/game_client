#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <string>

#include <pthread.h>

//#define HOST_ADDR "0.0.0.0"
#define HOST_ADDR "134.122.89.177"
#define HOST_PORT 1903
#define BUFFER_SIZE 4096

using namespace std;


void *listener(void *arg);

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
    pthread_create(&listen_thread, NULL, listener, (void *)&sockfd);
    
    char temp[] = "hello\r\n\0";
    send(sockfd, temp, strlen(temp), 0);
    
    char buffer_tx[BUFFER_SIZE];
    
    while (1) {
        memset(&buffer_tx, 0, BUFFER_SIZE);
        
        scanf(" %[^\n]", buffer_tx);

        send(sockfd, buffer_tx, strlen(buffer_tx), 0);
        
        if (strcmp(buffer_tx, "quit") == 0) {
            break;
        }
    }
    
    close(sockfd);
    
    return 0;
}

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



