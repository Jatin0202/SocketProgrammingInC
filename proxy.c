/* 
    Proxy Side Server
    filename portno(for proxy) dns_server_ipaddress dns_server_portno
    argv[0]  argv[1]           argv[2]              argv[3]
    
    To run code    
    Step 1: "gcc proxy.c -o proxy" This will generate a object file proxy
    Step 2: "./proxy proxy_server_portno dns_server_ipaddress dns_server_portno"
            ex. "./proxy 2000 127.0.0.1 3000"
                (Assuming Proxy.c is running on port no. 2000, dnsServer is running on same pc(loopback 127.0.0.1) and dnsServer is running on port 3000)

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

// Defining Structure of Local Cache(Priority Based)
struct LocalCache{
    char IP_Address[1024];
    char DomainName[1024];
    int priority;
};
struct LocalCache Cache[3];
    
// Defining error function
void error(const char *msg){
    perror(msg);
    exit(1);
}

// Printing Cache
void PrintCache(){
    printf("Proxy Server Cache(upper one have high priorities)\n");
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            if(i==Cache[j].priority){
                printf("%s : %s\n", Cache[j].IP_Address, Cache[j].DomainName);
            }
        }
    }
    printf("\n\n");
}

// Define Cache
void DefineCache(){
    // Defining Local Cache(Less Priority number, more is preferred)(Initial Cache, wiil be updated time to time)
    char IP0[1024]= "0.0.0.0";
    strcpy(Cache[0].IP_Address, IP0);
    char IP1[1024]= "1.1.1.1";
    strcpy(Cache[1].IP_Address, IP1);
    char IP2[1024]= "2.2.2.2";
    strcpy(Cache[2].IP_Address, IP2);
    char DN0[1024]= "www.google.com";
    strcpy(Cache[0].DomainName, DN0);
    char DN1[1024]= "www.fb.com";
    strcpy(Cache[1].DomainName, DN1);
    char DN2[1024]= "www.youtube.com";
    strcpy(Cache[2].DomainName, DN2);
    Cache[0].priority= 0;
    Cache[1].priority= 1;
    Cache[2].priority= 2;
}

// Searching Local Cache
int SearchLocalCache(char *buffer, char *response){
    char Request[1024];
    strcpy(Request, buffer+2);
    Request[strlen(Request)-1]= '\0';

    for(int i=0;i<3;i++){
        if((strcmp(Cache[i].DomainName, Request)==0) || (strcmp(Cache[i].IP_Address, Request))==0){
            if(strcmp(Cache[i].DomainName, Request)==0)
                strcpy(response, Cache[i].IP_Address);
            if(strcmp(Cache[i].IP_Address, Request)==0)
                strcpy(response, Cache[i].DomainName);
            Cache[i].priority= 0;
            Cache[(i+1)%3].priority= 2;
            Cache[(i+2)%3].priority= 1;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]){

    // If argc<4 (either port no or ip address of dnsserver, portno of proxyserver is not provided), therefore error
    if(argc!=4){
        fprintf(stderr, "Usage: ./%s port(proxy) hostname port(dns_server)\n", argv[0]);
        exit(1);
    }
    // Defining Initial Cache
    DefineCache();

    // Initially as a Server (Proxy Server will look in its cache)
    int proxy_sockfd, new_sockfd, portno, msg_rcvd, n, dns_portno, dns_sockfd;
    struct sockaddr_in proxy_server_addr, client_addr, dns_server_addr;
    struct hostent *dns_server;
    char buffer[1024], ip_address[1024], domain_name[1024];
    socklen_t clilen;
    FILE *database;

    // Setting DNS Server
    dns_portno= atoi(argv[3]);
    dns_server= gethostbyname(argv[2]);
    bzero((char *) &dns_server_addr, sizeof(dns_server_addr));
    dns_server_addr.sin_family= AF_INET;
    bcopy((char *) dns_server->h_addr, (char *) &dns_server_addr.sin_addr.s_addr, dns_server->h_length);
    dns_server_addr.sin_port= htons(dns_portno);
    dns_sockfd= socket(AF_INET, SOCK_STREAM, 0);
    close(dns_sockfd);

    // Creating socket file descriptor for listening at the Proxy Server
    proxy_sockfd= socket(AF_INET, SOCK_STREAM, 0);
    if(proxy_sockfd<0){
        error("Error Opening Socket at DNS Server");
    }

    // Setting Proxy Server IP Address and port    
    bzero((char *) &proxy_server_addr, sizeof(proxy_server_addr));
    portno= atoi(argv[1]);
    proxy_server_addr.sin_family= AF_INET;
    proxy_server_addr.sin_addr.s_addr= INADDR_ANY;
    proxy_server_addr.sin_port= htons(portno);

    // Attaching Socket to Client 
    if(bind(proxy_sockfd, (struct sockaddr *) &proxy_server_addr, sizeof(proxy_server_addr))<0){
        error("Binding Failed");
    }

    // Listening on the created socket
    listen(proxy_sockfd, 5);
    clilen= sizeof(client_addr);

    while(1){
        // Printing Cache
        PrintCache();
        // Empty current buffer
        bzero(buffer, 1024);

        // Accept a connection
        new_sockfd= accept(proxy_sockfd, (struct sockaddr *) &client_addr, &clilen);
        if(new_sockfd<0){
            error("Error on Accept");
        }
        
        // Receive a message from client side
        msg_rcvd= read(new_sockfd, buffer, 1024);
        if(msg_rcvd<0){
            error("Error on reading");
        }
        printf("Client Request:   Type %c- %s", buffer[0], buffer+1);
        
        if(buffer[0]=='1' || buffer[0]=='2'){
            char response[1024];
            // Looking in Local Cache whether it can handle request or it has to ask from DNS Server
            if(SearchLocalCache(buffer, response)==0){
                
                // Creating DNS Socket and connection
                dns_sockfd= socket(AF_INET, SOCK_STREAM, 0);
                if(dns_sockfd<0){
                    error("ERROR opening socket");
                }
                if(connect(dns_sockfd, (struct sockaddr *) &dns_server_addr, sizeof(dns_server_addr))<0){
                    error("Connection failed");
                }

                // Request to DNS Server
                bzero(response, 1024);
                n= send(dns_sockfd, buffer, strlen(buffer), 0);
                n= read(dns_sockfd, response, 1024);
                close(dns_sockfd);

                printf("\n%s\n", response);
                // Update Cache
                if(response[0]!='S'){
                    for(int i=0;i<3;i++){
                        if(Cache[i].priority==0){
                            Cache[i].priority=1;
                        }
                        else if(Cache[i].priority==1){
                            Cache[i].priority=2;
                        }
                        else if(Cache[i].priority==2){
                            Cache[i].priority=0;
                            if(buffer[0]=='1'){
                                buffer[strlen(buffer)-1]= '\0';
                                strcpy(Cache[i].DomainName, buffer+2);
                                strcpy(Cache[i].IP_Address, response);
                            }   
                            if(buffer[0]=='2'){
                                buffer[strlen(buffer)-1]= '\0';
                                strcpy(Cache[i].DomainName, response);
                                strcpy(Cache[i].IP_Address, buffer+2);
                            }
                        }
                    }
                    printf("Client request is fulfilled by DNS Server, Cache is updated\n");
                }
                else{
                    printf("Sorry DNS Server's DataBase can't fulfil client request\n");
                }

                strcpy(buffer, response);
                n= send(new_sockfd, buffer, strlen(buffer), 0);
                close(new_sockfd);
            }
            else{
                strcpy(buffer, response);
                n= send(new_sockfd, buffer, strlen(buffer), 0);
                printf("Client request is fulfilled by Proxy Cache\n");
                close(new_sockfd);
            }
        }
        else{
            bzero(buffer, 1024);
			char response[1024]= "Sorry wrong request";
            strcpy(buffer, response);
            n= send(new_sockfd, buffer, strlen(buffer), 0);
            close(new_sockfd);
        }

        printf("\n\n");
    }
    
    close(proxy_sockfd);
    return 0;
}