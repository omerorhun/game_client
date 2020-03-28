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
        
        if (input == 1) {
            // send "fb login" request
            
            printf("sending login with facebook request...\n");
            
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAMlKrYd5EWok0NtT4pgaAoZCvg6TsEjDiptJE2ZCEdAJ3sPdi6WEbFAl19cYAKWNioeWAD9Cj5Y9b3LN0AnN0KMQcpNKlDsFZAHh1CKCcn42pEoccVfqhK6hgi77wLRYSJYNZAjCFkmTCNXWewgtFiQwJu8UQgl01wZBPsHQLhfMOy6PGZAjg8G4DOScZCs31LRCYEHenIP&data_access_expiration_time=1592853060&expires_in=6539
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAPueEjX2jgNYqc3br7agj0rRYOmWMtyLsbG8N0Xous0S0VfYcikloxLBVZClD0ZCZBlOvWz5TbG3n4LRp9L8wWRWMHrSlaw0r4dGwDOX5bCqDwN3VgVkrPmUQlKQSNTH1b1XrDNqC2No7C8cRafItvxRKxOVAai6ZBBTpSsUF3exoyVn8cDJa65fIFYpjRd7QZBNaK2KH&data_access_expiration_time=1592873759&expires_in=3840
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAJhvlgDMSTqql4enth7VKgiOIqOUYYuetZC3vGpG3dEZBdNofZCCvvE1M70hSq8iZAzjzAZA1dOdTzsPfhmNiRVyEARisNNBDLwIHGsuBaRPLIo74SPSVqxxlZAhFZBzoEFbZClkW498lpGZBzxbyKmBgxqNyDiEMOs9ieM1HUrl7FHeKBZCFyEAHfWCbZCltN9E4GWBUKWIwB9&data_access_expiration_time=1593101748&expires_in=6251
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAAOp0ZBe7UJXUfM7bDujpA5CJFviJmxDDbU7VFZBGjMO0yY4Y8nujx4nVYQ47IymtA4ZBaZAL4Vbhz0UNWupYjFzziYjmNgwDahbeUCjUgw6ZCeKdZBNr3UKf89jf1Hh1lErBq1zMd03FwReJ3ysVW1jQisCthqavJAE9c4svYZB2SJhWD9t0CtsURfqSuKifNpopo14yzw&data_access_expiration_time=1593109134&expires_in=6065
            //https://www.facebook.com/connect/login_success.html#access_token=EAAJQZBZANTOG0BAO09VDNAdOZANzz7MmIgSieysG8MpKEAcBS6feRZCyoJSBnIsUd0eZANw0EGbPVQ1GhYESYWSKsG837EIDGO9wVnKzKKxjaX33l0ZCiyZBAtAZBuSZApE6kCpqQoDYPKpjomtxAJMRBZAwh2Ma9MZAZBhdHZCBhboCVOzgJzkGmFFRcH0sdZB5ni1vlOeG1tQYPPZBts4NH1Jm7le&data_access_expiration_time=1593128305&expires_in=4895
            string access_token = "EAAJQZBZANTOG0BAO09VDNAdOZANzz7MmIgSieysG8MpKEAcBS6feRZCyoJSBnIsUd0eZANw0EGbPVQ1GhYESYWSKsG837EIDGO9wVnKzKKxjaX33l0ZCiyZBAtAZBuSZApE6kCpqQoDYPKpjomtxAJMRBZAwh2Ma9MZAZBhdHZCBhboCVOzgJzkGmFFRcH0sdZB5ni1vlOeG1tQYPPZBts4NH1Jm7le";
            Requests request(sockfd);
            request.send_request(REQ_FB_LOGIN, access_token);
        }
        else if (input == 2) {
            // send "get online users" request
            
            printf("sending get online users request...\n");
            
            Requests request(sockfd);
            request.send_request(REQ_GET_ONLINE_USERS, "");
        }
        
        Requests response(sockfd);
        response.get_response();
        
        close(sockfd);
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
    cout << "2: send 'get online clients' request" << endl << endl;
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


