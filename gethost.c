#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int i = 0;
    struct hostent *h_ent = NULL;
    struct in_addr **addr_list = NULL;

    if (argc != 2) {
        perror("Usage: file <domain>\n");
        return EXIT_FAILURE;
    }
    h_ent = gethostbyname(argv[1]);
    if (h_ent == NULL) {
        printf("gethost failed .%s", hstrerror(h_errno));
        return EXIT_FAILURE;
    }

    printf("Host Name: %s\n", h_ent->h_name);
    addr_list = (struct in_addr **) h_ent->h_addr_list;
    for (i=0; addr_list[i] != NULL; i++) {
        printf("IP Address: %s\n", inet_ntoa(*addr_list[i]));
    }
    return 0;
}
