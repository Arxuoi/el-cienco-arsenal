#include "../../include/elcienco.h"

void random_ip(char *ip_buffer) {
    snprintf(ip_buffer, 16, "%d.%d.%d.%d",
             rand() % 255, rand() % 255, rand() % 255, rand() % 255);
}

void random_private_ip(char *ip_buffer) {
    // Generate RFC 1918 private IP addresses
    int class = rand() % 3;
    
    switch(class) {
        case 0: // 10.0.0.0/8
            snprintf(ip_buffer, 16, "10.%d.%d.%d",
                     rand() % 255, rand() % 255, rand() % 255);
            break;
        case 1: // 172.16.0.0/12
            snprintf(ip_buffer, 16, "172.%d.%d.%d",
                     16 + (rand() % 16), rand() % 255, rand() % 255);
            break;
        case 2: // 192.168.0.0/16
            snprintf(ip_buffer, 16, "192.168.%d.%d",
                     rand() % 255, rand() % 255);
            break;
    }
}

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

int resolve_hostname(const char *hostname, char *ip_buffer) {
    struct hostent *he;
    struct in_addr **addr_list;
    
    he = gethostbyname(hostname);
    if (he == NULL) {
        return -1;
    }
    
    addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[0] != NULL) {
        strcpy(ip_buffer, inet_ntoa(*addr_list[0]));
        return 0;
    }
    
    return -1;
}
