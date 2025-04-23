#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>

#if BYTE_ORDER == LITTLE_ENDIAN
# define ODDBYTE(v) (v)
#elif BYTE_ORDER == BIG_ENDIAN          
# define ODDBYTE(v) ((unsigned short)(v) << 8)
#else
# define ODDBYTE(v) htons((unsigned short)(v) << 8)
#endif      


char *pr_addr(void *sa, socklen_t salen)
{                                       
    static char buffer[4096] = "";
    static struct sockaddr_storage last_sa = { 0 };
    static socklen_t last_salen = 0;
    char address[128];

    if (salen == last_salen && !memcmp(sa, &last_sa, salen))
        return buffer;

    memcpy(&last_sa, sa, (last_salen = salen));
    getnameinfo(sa, salen, address, sizeof(address), NULL, 0, NI_NUMERICHOST);
    snprintf(buffer, sizeof(buffer), "%s", address);

    return buffer;
}

unsigned short in_cksum(const unsigned short *addr, register int len, unsigned short csum)
{
    register int nleft = len;
    const unsigned short *w = addr;
    register unsigned short answer;
    register int sum = csum;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;                                 
    }

    if (nleft == 1)
        sum += ODDBYTE(*(unsigned char *)w);

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;                      
}

int main(int argc, char **argv) {
    int sock, alen;
    struct sockaddr_in source = { .sin_family = AF_INET };
    struct sockaddr_in dst;

    if (argc != 2) {
        printf("Usage: %s <IP-Address>\n", argv[0]);
        _exit(EXIT_FAILURE);
    }

    memset((char *)&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    if (inet_aton(argv[1], &dst.sin_addr) == 0) {
        fprintf(stderr, "The first argument must be an IP-Address\n");
        exit(1);
    }
    dst.sin_port = htons(1025);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sock == -1) {
        perror("-Error creating socket-");
        exit(1);
    }

    if (getsockname(sock, (struct sockaddr*)&source, &alen) == -1) {
        perror("getsockname");
        exit(2);
    }

    int datalen = 56;
    int MAXIPLEN = 60;
    int MAXICMPLEN = 76;
    unsigned char *packet;
    struct icmphdr *icp;
    int ntransmitted = 0;
    int num_packets = 5;  // Number of packets to send

    int packlen = datalen + MAXIPLEN + MAXICMPLEN;
    if (!(packet = (unsigned char *)malloc((unsigned int)packlen))) {
        fprintf(stderr, "ping: out of memory.\n");
        exit(2);
    }

    for (int i = 0; i < num_packets; i++) {
        icp = (struct icmphdr *)packet;
        icp->type = ICMP_ECHO;
        icp->code = 0;
        icp->checksum = 0;
        icp->un.echo.sequence = htons(ntransmitted + 1);

        int cc = datalen + 8;
        icp->checksum = in_cksum((unsigned short *)icp, cc, 0);

        int sent_bytes = sendto(sock, icp, cc, 0, (struct sockaddr*)&dst, sizeof(dst));
        printf("Sent %d bytes\n", sent_bytes);

        struct msghdr msg;
        int polling;
        char addrbuf[128];
        struct iovec iov;

        iov.iov_base = (char *)packet;
        iov.iov_len = packlen;

        memset(&msg, 0, sizeof(msg));
        msg.msg_name = addrbuf;
        msg.msg_namelen = sizeof(addrbuf);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        polling = MSG_WAITALL;
        cc = recvmsg(sock, &msg, polling);
        if (cc < 0) {
            perror("Error in recvmsg");
            exit(1);
        }

        __u8 *buf = msg.msg_iov->iov_base;
        struct icmphdr *icp_reply;
        int csfailed;
        icp_reply = (struct icmphdr *)buf;
        csfailed = in_cksum((unsigned short *)icp_reply, cc, 0);
        if (csfailed) {
            printf("(BAD CHECKSUM)");
            exit(1);
        }

        struct sockaddr_in *from = msg.msg_name;
        if (icp_reply->type == ICMP_ECHOREPLY) {
            printf("%s\n", pr_addr(from, sizeof *from));
            printf("Reply of %d bytes received\n", cc);
            printf("icmp_seq = %u\n", ntohs(icp_reply->un.echo.sequence));
        } else {
            printf("Not an ICMP_ECHOREPLY\n");
        }

        ntransmitted++;
        sleep(1);  // Wait for 1 second before sending the next packet
    }

    free(packet);
    close(sock);
    return 0;
}
