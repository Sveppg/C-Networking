#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <errno.h>

// ICMP-Header für macOS
struct icmp_packet {
    struct icmp hdr;
    char data[56];  // ICMP-Payload
};

// Checksumme berechnen
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

// BPF-Gerät öffnen (für ICMP-Empfang)
int open_bpf() {
    char dev[11];
    int fd;
    for (int i = 0; i < 99; i++) {
        snprintf(dev, sizeof(dev), "/dev/bpf%d", i);
        fd = open(dev, O_RDWR);
        if (fd != -1)
            return fd;
        if (errno != EBUSY)
            break;
    }
    return -1;
}

// ICMP-Ping senden und Antwort lesen
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ziel-IP
    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &target.sin_addr) != 1) {
        perror("Invalid IP address");
        return EXIT_FAILURE;
    }

    // UDP-Socket für ICMP-Send
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Socket error");
        return EXIT_FAILURE;
    }

    // BPF-Gerät öffnen
    int bpf = open_bpf();
    if (bpf < 0) {
        perror("Failed to open BPF device");
        return EXIT_FAILURE;
    }

    // ICMP-Ping-Paket vorbereiten
    struct icmp_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.hdr.icmp_type = ICMP_ECHO;
    packet.hdr.icmp_code = 0;
    packet.hdr.icmp_id = htons(getpid());
    packet.hdr.icmp_seq = htons(1);
    memset(packet.data, 0x42, sizeof(packet.data));  // Dummy-Daten
    packet.hdr.icmp_cksum = checksum(&packet, sizeof(packet));

    // ICMP-Paket senden
    ssize_t sent = sendto(sock, &packet, sizeof(packet), 0,
                          (struct sockaddr *)&target, sizeof(target));
    if (sent < 0) {
        perror("Sendto failed");
        return EXIT_FAILURE;
    }
    printf("Ping sent to %s\n", argv[1]);

    // Antwort empfangen
    struct ip *ip_hdr;
    struct icmp *icmp_hdr;
    char buf[1024];

    while (1) {
        ssize_t received = read(bpf, buf, sizeof(buf));
        if (received < 0) {
            perror("Read failed");
            return EXIT_FAILURE;
        }

        ip_hdr = (struct ip *)(buf + 2);
        if (ip_hdr->ip_p != IPPROTO_ICMP)
            continue;

        icmp_hdr = (struct icmp *)(buf + 2 + (ip_hdr->ip_hl << 2));
        if (icmp_hdr->icmp_type == ICMP_ECHOREPLY && icmp_hdr->icmp_id == packet.hdr.icmp_id) {
            printf("Received ICMP reply from %s\n", argv[1]);
            break;
        }
    }

    close(sock);
    close(bpf);
    return EXIT_SUCCESS;
}
