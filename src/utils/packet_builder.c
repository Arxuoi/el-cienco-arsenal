#include "../../include/elcienco.h"
#include "../../include/protocols.h"

void build_ip_header(struct ip_header *iph, const char *src_ip, const char *dst_ip, 
                     int protocol, int payload_len) {
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct ip_header) + payload_len);
    iph->id = htons(rand() % 65535);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = protocol;
    iph->check = 0;
    iph->saddr = inet_addr(src_ip);
    iph->daddr = inet_addr(dst_ip);
    
    iph->check = checksum((unsigned short *)iph, sizeof(struct ip_header));
}

void build_tcp_header(struct tcp_header *tcph, int src_port, int dst_port, int flags) {
    tcph->source = htons(src_port);
    tcph->dest = htons(dst_port);
    tcph->seq = htonl(rand());
    tcph->ack_seq = 0;
    tcph->res1 = 0;
    tcph->doff = 5;
    tcph->fin = (flags & 0x01) ? 1 : 0;
    tcph->syn = (flags & 0x02) ? 1 : 0;
    tcph->rst = (flags & 0x04) ? 1 : 0;
    tcph->psh = (flags & 0x08) ? 1 : 0;
    tcph->ack = (flags & 0x10) ? 1 : 0;
    tcph->urg = 0;
    tcph->ece = 0;
    tcph->cwr = 0;
    tcph->window = htons(65535);
    tcph->check = 0;
    tcph->urg_ptr = 0;
}

void build_tcp_syn_packet(char *packet, char *src_ip, char *dst_ip, 
                           int src_port, int dst_port) {
    struct ip_header *iph = (struct ip_header *)packet;
    struct tcp_header *tcph = (struct tcp_header *)(packet + sizeof(struct ip_header));
    struct tcp_pseudo psh;
    
    // Build headers
    build_ip_header(iph, src_ip, dst_ip, PROTO_TCP, sizeof(struct tcp_header));
    build_tcp_header(tcph, src_port, dst_port, 0x02); // SYN flag
    
    // TCP checksum with pseudo header
    psh.source_address = inet_addr(src_ip);
    psh.dest_address = inet_addr(dst_ip);
    psh.placeholder = 0;
    psh.protocol = PROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcp_header));
    
    char pseudo_packet[sizeof(struct tcp_pseudo) + sizeof(struct tcp_header)];
    memcpy(pseudo_packet, &psh, sizeof(struct tcp_pseudo));
    memcpy(pseudo_packet + sizeof(struct tcp_pseudo), tcph, sizeof(struct tcp_header));
    
    tcph->check = checksum((unsigned short *)pseudo_packet, sizeof(pseudo_packet));
}

void build_udp_packet(char *packet, char *src_ip, char *dst_ip, 
                       int src_port, int dst_port) {
    struct ip_header *iph = (struct ip_header *)packet;
    struct udp_header *udph = (struct udp_header *)(packet + sizeof(struct ip_header));
    
    int payload_len = MAX_PACKET_SIZE - sizeof(struct ip_header) - sizeof(struct udp_header);
    
    // Fill payload with random data
    char *payload = packet + sizeof(struct ip_header) + sizeof(struct udp_header);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = rand() % 256;
    }
    
    // Build UDP header
    udph->source = htons(src_port);
    udph->dest = htons(dst_port);
    udph->len = htons(sizeof(struct udp_header) + payload_len);
    udph->check = 0; // Optional for IPv4
    
    // Build IP header
    build_ip_header(iph, src_ip, dst_ip, PROTO_UDP, 
                    sizeof(struct udp_header) + payload_len);
}

void build_icmp_echo_packet(char *packet, char *src_ip, char *dst_ip) {
    struct ip_header *iph = (struct ip_header *)packet;
    struct icmp_header *icmph = (struct icmp_header *)(packet + sizeof(struct ip_header));
    
    icmph->type = 8; // Echo request
    icmph->code = 0;
    icmph->id = htons(getpid() & 0xFFFF);
    icmph->sequence = htons(rand() % 65535);
    icmph->check = 0;
    icmph->check = checksum((unsigned short *)icmph, sizeof(struct icmp_header));
    
    build_ip_header(iph, src_ip, dst_ip, PROTO_ICMP, sizeof(struct icmp_header));
}
