#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

static union {
    uint8_t buf[2];
    uint16_t uint16;
} endian = { {0x00, 0x3a} };

#define LITTLE_ENDIANNESS ((char)endian.uint16 == 0x00)
#define BIG_ENDIANNESS ((char)endian.uint16 == 0x3a)

int main(int argc, char *argv[])
{
    uint16_t host_short_val = 0x01;
    uint16_t net_short_val = 0;
    uint32_t host_long_val = 0x02;
    uint32_t net_long_val = 0;

    net_short_val  = htons(host_short_val);
    net_long_val   = htonl(host_long_val);
    host_short_val = htons(net_short_val);
    host_long_val  = htonl(net_long_val);

    if (LITTLE_ENDIANNESS) {
        printf("On Little Endian Machine:\n");
    } else {
        printf("On Big Endian Machine\n");
    }
    printf("htons(0x%x) = 0x%x\n", host_short_val, net_short_val);
    printf("htonl(0x%x) = 0x%x\n", host_long_val, net_long_val);

    host_short_val = htons(net_short_val);
    host_long_val  = htonl(net_long_val);

    printf("ntohs(0x%x) = 0x%x\n", net_short_val, host_short_val);
    printf("ntohl(0x%x) = 0x%x\n", net_long_val, host_long_val);
    return 0;
}