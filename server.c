/* A simple server in the internet domain using TCP 
    The port number is passed as an argument 
    DNS Server side C Program */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

int SearchDatabase(char *buffer, char *Response){
    FILE *database;
    char DomainName[1024], IP_Address[1024], Request[1024];
    database= fopen("database.txt", "r");
    strcpy(Request, buffer+2);
    Request[strlen(Request)-1]= '\0';

    while(fscanf(database, "%[^,],%[^\n]\n", DomainName, IP_Address) != -1){
        if(strcmp(DomainName, Request)==0){
            strcpy(Response, IP_Address);
            fclose(database);
            return 1;
        }
        if(strcmp(IP_Address, Request)==0){
            strcpy(Response, DomainName);
            fclose(database);
            return 1;
        }
	}
    fclose(database);
    return 0;
}

void printDatabase(){
	FILE *database;
    char DomainName[1024], IP_Address[1024], Request[1024];
    database= fopen("database.txt", "r");
    while(fscanf(database, "%[^,],%[^\n]\n", DomainName, IP_Address) != -1){
        printf("%s           %s\n",IP_Address,DomainName);
	}
    fclose(database);
}

int main(int argc, char *argv[]){
    if(argc!=2){
        fprintf(stderr, "Port number not provided. Program terminated");
        exit(1);
    }
    
    printf("Server Started, Waiting for request\n\nMy Database(used File I/O to print this database)\n");
    printDatabase();
    printf("\n\n\n\n");
    
    int dns_sockfd, new_sockfd, portno, msg_rcvd, n;
    struct sockaddr_in dns_server_addr, client_addr;
    char buffer[1024], ip_address[1024], domain_name[1024];
    socklen_t clilen;
    

    // Creating socket file descriptor for listening at the DNS Server
    dns_sockfd= socket(AF_INET, SOCK_STREAM, 0);
    if(dns_sockfd<0){
        error("Error Opening Socket at DNS Server");
    }

    // Setting DNS Server IP Address and port    
    bzero((char *) &dns_server_addr, sizeof(dns_server_addr));
    portno= atoi(argv[1]);
    dns_server_addr.sin_family= AF_INET;
    dns_server_addr.sin_addr.s_addr= INADDR_ANY;
    dns_server_addr.sin_port= htons(portno);
	
    // Attaching Socket to DNS Server 
    if(bind(dns_sockfd, (struct sockaddr *) &dns_server_addr, sizeof(dns_server_addr))<0){
        error("Binding Failed");
    }
	
    // Listening on the created socket
    listen(dns_sockfd, 5);
    clilen= sizeof(client_addr);
	
    // Loop(from Server Side)
    while(1){
        // Accept a connection
        new_sockfd= accept(dns_sockfd, (struct sockaddr *) &client_addr, &clilen);
        if(new_sockfd<0){
            error("Error on Accept");
        }


        // Empty current buffer
        bzero(buffer, 1024);

        // Receive a message from client side(Proxy as a client)
        msg_rcvd= read(new_sockfd, buffer, 1024);
        if(msg_rcvd<0){
            error("Error on reading");
        }
        printf("Client Request(Proxy):   Type %c- %s", buffer[0], buffer+1);
        
        // Searching Database
        char Response[1024];
        if(!SearchDatabase(buffer, Response)){
            char A[1024]= "Sorry DNS Server's DataBase can't fulfil your request";
            strcpy(Response, A);
        }
        printf("My Response: %s", Response);
        printf("\n_________________________________________________________________\n");

        strcpy(buffer, Response);
        send(new_sockfd, buffer, strlen(buffer), 0);

        
        close(new_sockfd);
    }
    close(dns_sockfd);
    
    return 0;
}
