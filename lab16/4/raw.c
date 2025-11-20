#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <time.h>

#define SIZE_BUF 65536

 static void print_time() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%F %T", tm);
    printf("[%s] ", buf);
 }

 static void hex_dump(const unsigned char *data, int len) {
    for (int i = 0; i < len; ++i) {
        if (i % 16 == 0) printf("\n%04x ", i);
        printf("%02x ", data[i]);
    }
    printf("\n");
 }

 int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage %s", argv[0]);
        return 1;
    }

    int proto = 0;
    if (strcmp(argv[1], "udp") == 0) proto = IPPROTO_UDP;
    else if (strcmp(argv[1], "tcp") == 0) proto = IPPROTO_TCP;
    else {
        fprintf(stderr, "Not UDP or TCP\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_RAW, proto);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    printf("Raw socket created for protocol %s (proto = %d)\n", proto == IPPROTO_UDP ? "UDP" : "TCP", proto);

    unsigned char *buffer = malloc(SIZE_BUF);
    if (!buffer) {
        perror("malloc");
        close(sock);
        return 1;
    }

    while (1) {
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        ssize_t n = recvfrom(sock, buffer, SIZE_BUF, 0, (struct sockaddr*)&addr, &addrlen);
        
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recvfrom");
            break;
        }
        if (n == 0) continue;

        struct iphdr *ip = (struct iphdr*)buffer;
        size_t ip_header_len = ip->ihl * 4;
        char src_ip[INET_ADDRSTRLEN], dst_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip->saddr, src_ip, sizeof(src_ip));
        inet_ntop(AF_INET, &ip->saddr, dst_ip, sizeof(dst_ip));

        print_time();
        printf("Captured %zd bytes: %s -> %s (proto = %u, ihl = %zu bytes)\n", n, src_ip, dst_ip, ip->protocol, ip_header_len);

        if (ip->protocol == IPPROTO_TCP && proto == IPPROTO_TCP) {
            if (n < ip_header_len + sizeof(struct tcphdr)) {
                printf("too short for TCP\n");
                continue;
            }
            struct tcphdr *tcp = (struct tcphdr*)(buffer + ip_header_len);
            uint16_t sport = ntohl(tcp->source);
            uint16_t dport = ntohs(tcp->dest);
            int tcp_header_len = tcp->doff * 4;
            printf("TCP: %s:%u -> %s:%u seq = %u ack = %u hdr_len = %d flags = [%c%c%c%c%c%c]\n",
                    src_ip, sport, dst_ip, dport,
                    ntohl(tcp->seq), ntohl(tcp->ack_seq), tcp_header_len,
                    (tcp->urg ? 'U' : '-'), (tcp->ack ? 'A' : '-'),
                    (tcp->psh ? 'P' : '-'), (tcp->rst ? 'R' : '-'),
                    (tcp->syn ? 'S' : '-'), (tcp->fin ? 'F' : '-'));

            size_t payload_offset = ip_header_len + tcp_header_len;
            if (n > (ssize_t)payload_offset) {
                size_t payload_len = n - payload_offset;
                printf("Payload lenght: %zu bytes\n", payload_len);
                size_t to_print = payload_len < 128 ? payload_len : 128;
                hex_dump(buffer + payload_offset, to_print);
            } else printf("No payload\n");
        } else if (ip->protocol == IPPROTO_UDP && proto == IPPROTO_UDP) {
            if (n < ip_header_len + sizeof(struct udphdr)) {
                printf("too short for UDP\n");
                continue;
            }
            struct udphdr *udp = (struct udphdr*)(buffer + ip_header_len);
            uint16_t sport = ntohs(udp->source);
            uint16_t drop = ntohs(udp->dest);
            uint16_t udplen = ntohs(udp->len);
            printf("UDP %s:%u -> %s:%u lenght = %u\n", src_ip, sport, dst_ip, drop, udplen);

            size_t payload_offset = ip_header_len + sizeof(struct udphdr);
            if (n > (ssize_t)payload_offset) {
                size_t payload_len = n - payload_offset;
                printf("Payload lenght: %zu bytes\n", payload_len);
                size_t to_print = payload_len < 128 ? payload_len : 128;
                hex_dump(buffer + payload_offset, to_print);
            } else printf("No payload\n");
        } else printf("Packet protocol %u does not match sniffed protocol %d; skiping details\n", ip->protocol, proto);
        printf("\n");
        fflush(stdout);
    }

    free(buffer);
    close(sock);
    return 0;
 }