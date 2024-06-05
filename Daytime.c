#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 4096  // Define MAXLINE with a reasonable buffer size

int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in serveraddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <IP-Address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }
    
    memset(&serveraddr, 0, sizeof(serveraddr));  
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(13);
    
    if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }
    
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("Connect error");
        exit(EXIT_FAILURE);
    }
    
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0'; 
        if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(EXIT_FAILURE);
        }
    }
    
    if (n < 0) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    close(sockfd);  // Close the socket
    return 0;
}
