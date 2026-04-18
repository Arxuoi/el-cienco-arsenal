#include "../../include/elcienco.h"

int create_raw_socket(void) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        perror("Raw socket creation failed");
        return -1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("Failed to set IP_HDRINCL");
        close(sock);
        return -1;
    }
    
    return sock;
}

int create_tcp_socket(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("TCP socket creation failed");
        return -1;
    }
    
    // Set non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    return sock;
}

int create_udp_socket(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP socket creation failed");
        return -1;
    }
    
    // Increase send buffer
    int send_buf = 1024 * 1024; // 1MB
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buf, sizeof(send_buf));
    
    return sock;
}

int connect_with_timeout(int sock, struct sockaddr *addr, socklen_t addrlen, int timeout_sec) {
    // Set non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    int ret = connect(sock, addr, addrlen);
    
    if (ret < 0 && errno == EINPROGRESS) {
        fd_set fdset;
        struct timeval tv;
        
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        
        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;
        
        if (select(sock + 1, NULL, &fdset, NULL, &tv) > 0) {
            int error;
            socklen_t len = sizeof(error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
            
            if (error == 0) {
                // Restore blocking mode
                fcntl(sock, F_SETFL, flags);
                return 0;
            }
        }
    }
    
    // Restore blocking mode
    fcntl(sock, F_SETFL, flags);
    return -1;
}

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
    
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    
    if (len == 1)
        sum += *(unsigned char *)buf;
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    
    return result;
}
