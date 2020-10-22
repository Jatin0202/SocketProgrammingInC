/*  
    Client Side Code
    filename proxy_server_ipaddres proxy_server_portno
    argv[0]  argv[1]         argv[2]

    To run code 
    Step 1: "gcc client.c -o client" This will generate a object file client
    Step 2: "./client proxy_server_ipaddress proxy_server_portno"
            ex. "./client 127.0.0.1 2000"
                (Assuming for a loopback ip, and port number(on which proxy server is running) is 2000)
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

// Defining error function
void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){

    // If argc<3 (either port no or ip address of server is not provided), therefore error
    if(argc!=3){
        fprintf(stderr, "Usage: ./%s hostname port\n", argv[0]);
        exit(1);
    }
    
    // Printing Type and format of queries
    printf("There are 2 types of request(as mentioned in question)\nType1: DomainName(Request) and IP Address(Response)\nType2: IP Address(Request) and DomainName(Response)");
    printf("\n\nex. of Type 1 request: '1 www.google.com'\nex. of Type 2 request: '2 1.1.1.1'");
    printf("\n\nERROR: If your request start from anynumber except 1 or 2, there is a error from server side(No request of this type)");

    // Defining Variables
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1024];
    
    // Storing portnumber(argv[2]) in variable portno
    portno= atoi(argv[2]);    

    // Checking whether IP address of server is correctly provided
    server= gethostbyname(argv[1]);
    if(server==NULL){
        fprintf(stderr, "Error, no such host");
    }

    // Defining serv_addr 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family= AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port= htons(portno);

    while(1){

        // Creating socket for connection oriented protocol
        sockfd= socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd<0){
            error("ERROR opening socket");
        }

        // Checking connection
        if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0){
            error("Connection failed");
        }

    	printf("\n\n\nType your request: ");
        bzero(buffer, 1024);
        fgets(buffer, 1024, stdin);

        // Writing request from client side and checking there is no error in writing
        n= write(sockfd, buffer, strlen(buffer));
        if(n<0){
            error("Error on writing");
        }     

        bzero(buffer, 1024);

        // Reading response from buffer side adn checking there is no error in reading
        n= read(sockfd, buffer, 255);
        if(n<0){
            error("Error on reading");
        }     
        printf("Server Response: %s", buffer);

        // Closing Socket
        close(sockfd);
    }
    
    return 0;
    
}
