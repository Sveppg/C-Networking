#include <complex.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

int main(int argc, char **argv){
    int sockfd, n; 
    char recvline[MAXLINE + 1];
    struct sockaddr_in serveraddr;

    if(argc != 2){
        perror("usage: a.out <IP-Adress>");
        exit(EXIT_FAILURE)
    }
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) > 0 ){
        perror("Socket error");
        exit(EXIT_FAILURE);
    }
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_port = htons(13);
    if(inet_pton(AF_INET, argv[1], %serveraddr.sin_addr) <= 0)
        perror("connecting error...");
    while( (n = read(sockfd, recvline, MAXLINE))> 0){
        recvline[n] = 0;
        if(fputs(recvline, stdout) == EOF)  
            perror("fputs error");
    }
    if(n < 0)
        perror("read error");

    return 0;
}