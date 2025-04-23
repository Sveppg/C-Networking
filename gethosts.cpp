#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv){
    int i = 0;
    struct hostent *h_ent = NULL;
    struct in_addr  **addr_list = NULL;
    if(argc != 2){
        std::cerr << "Usage file <domain>" << std::endl;
        return EXIT_FAILURE;
    }
    h_ent = gethostbyname(argv[1]);
    if(h_ent == NULL){
        std::cout << "gethost failed" << hstrerror(h_errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Host Name: " << h_ent->h_name << std::endl;
    addr_list = (struct in_addr **) h_ent->h_addr_list;
    for(i=0; addr_list[i] !=NULL; i++){
        std::cout << "IP-Address: " << inet_ntoa(*addr_list[i]);
    }
    return 0;
}