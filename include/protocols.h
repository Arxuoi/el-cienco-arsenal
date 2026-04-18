#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <stdint.h>
#include <netinet/in.h>

// Ethernet header (14 bytes)
struct eth_header {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t eth_type;
} __attribute__((packed));

// IPv4 header (20 bytes)
struct ip_header {
    uint8_t ihl:4, version:4;
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
} __attribute__((packed));

// TCP header (20-60 bytes)
struct tcp_header {
    uint16_t source;
    uint16_t dest;
    uint32_t seq;
    uint32_t ack_seq;
    uint8_t res1:4, doff:4;
    uint8_t fin:1, syn:1, rst:1, psh:1, ack:1, urg:1, ece:1, cwr:1;
    uint16_t window;
    uint16_t check;
    uint16_t urg_ptr;
} __attribute__((packed));

// TCP pseudo header (for checksum)
struct tcp_pseudo {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
} __attribute__((packed));

// UDP header (8 bytes)
struct udp_header {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
} __attribute__((packed));

// UDP pseudo header
struct udp_pseudo {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
} __attribute__((packed));

// ICMP header (8 bytes)
struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t check;
    uint16_t id;
    uint16_t sequence;
} __attribute__((packed));

// DNS header (12 bytes)
struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} __attribute__((packed));

// HTTP methods
#define HTTP_GET     1
#define HTTP_POST    2
#define HTTP_HEAD    3
#define HTTP_PUT     4
#define HTTP_DELETE  5

// Attack protocol types
#define PROTO_TCP    6
#define PROTO_UDP    17
#define PROTO_ICMP   1
#define PROTO_IGMP   2

// Packet building flags
#define PKT_TCP_SYN  0x01
#define PKT_TCP_ACK  0x02
#define PKT_TCP_RST  0x04
#define PKT_TCP_PSH  0x08
#define PKT_UDP      0x10
#define PKT_ICMP_ECHO 0x20

// DNS record types
#define DNS_A        1
#define DNS_NS       2
#define DNS_CNAME    5
#define DNS_SOA      6
#define DNS_PTR      12
#define DNS_MX       15
#define DNS_TXT      16
#define DNS_AAAA     28
#define DNS_ANY      255

#endif // PROTOCOLS_H
