#include "../include/elcienco.h"
#include "../include/api.h"
#include "routes.h"

// External variables from main.c
extern volatile int attack_running;
extern pthread_t attack_threads[MAX_THREADS];
extern AttackConfig current_attack;
extern time_t attack_start_time;

// Global route table definition
Route route_table[] = {
    {"/attack",  HTTP_METHOD_POST,   handle_attack},
    {"/stop",    HTTP_METHOD_POST,   handle_stop},
    {"/status",  HTTP_METHOD_GET,    handle_status},
    {"/methods", HTTP_METHOD_GET,    handle_methods},
    {"/config",  HTTP_METHOD_GET,    handle_config},
    {"/config",  HTTP_METHOD_POST,   handle_config},
    {"",         HTTP_METHOD_UNKNOWN, NULL}  // Sentinel
};

// Parse HTTP endpoint from path
ApiEndpoint parse_endpoint(const char *path) {
    if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        return ENDPOINT_ROOT;
    } else if (strcmp(path, "/attack") == 0) {
        return ENDPOINT_ATTACK;
    } else if (strcmp(path, "/stop") == 0) {
        return ENDPOINT_STOP;
    } else if (strcmp(path, "/status") == 0) {
        return ENDPOINT_STATUS;
    } else if (strcmp(path, "/methods") == 0) {
        return ENDPOINT_METHODS;
    } else if (strcmp(path, "/config") == 0) {
        return ENDPOINT_CONFIG;
    }
    return ENDPOINT_UNKNOWN;
}

// Parse HTTP method string
HttpMethod parse_http_method(const char *method) {
    if (strcmp(method, "GET") == 0) {
        return HTTP_METHOD_GET;
    } else if (strcmp(method, "POST") == 0) {
        return HTTP_METHOD_POST;
    } else if (strcmp(method, "PUT") == 0) {
        return HTTP_METHOD_PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        return HTTP_METHOD_DELETE;
    }
    return HTTP_METHOD_UNKNOWN;
}

// Parse raw HTTP request
void parse_http_request(const char *raw_request, ApiRequest *req) {
    char method_str[16] = {0};
    char path_str[256] = {0};
    char version_str[16] = {0};
    
    // Parse request line
    sscanf(raw_request, "%15s %255s %15s", method_str, path_str, version_str);
    
    req->method = parse_http_method(method_str);
    req->endpoint = parse_endpoint(path_str);
    req->header_count = 0;
    
    // Find body (after \r\n\r\n)
    char *body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        strncpy(req->body, body_start, API_BUFFER_SIZE - 1);
        req->body[API_BUFFER_SIZE - 1] = '\0';
        req->content_length = strlen(req->body);
    } else {
        req->body[0] = '\0';
        req->content_length = 0;
    }
    
    // Parse headers (simplified)
    char *headers_start = strstr(raw_request, "\r\n");
    if (headers_start) {
        headers_start += 2;
        char *line = headers_start;
        char *next_line;
        
        while (line && *line != '\r' && *line != '\n' && req->header_count < 10) {
            next_line = strstr(line, "\r\n");
            if (next_line) {
                size_t len = next_line - line;
                if (len < 256) {
                    strncpy(req->headers[req->header_count], line, len);
                    req->headers[req->header_count][len] = '\0';
                    req->header_count++;
                }
                line = next_line + 2;
            } else {
                break;
            }
        }
    }
}

// Build HTTP response
void build_http_response(ApiResponse *res, char *buffer, size_t *size) {
    const char *status_text = "OK";
    
    switch (res->status_code) {
        case HTTP_OK: status_text = "OK"; break;
        case HTTP_BAD_REQUEST: status_text = "Bad Request"; break;
        case HTTP_NOT_FOUND: status_text = "Not Found"; break;
        case HTTP_METHOD_NOT_ALLOWED: status_text = "Method Not Allowed"; break;
        case HTTP_INTERNAL_ERROR: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }
    
    *size = snprintf(buffer, API_BUFFER_SIZE,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        res->status_code, status_text,
        res->content_type,
        strlen(res->body),
        res->body
    );
}

// JSON utilities
void json_escape(char *dest, const char *src, size_t max_len) {
    size_t i = 0, j = 0;
    
    while (src[i] && j < max_len - 1) {
        switch (src[i]) {
            case '"': dest[j++] = '\\'; dest[j++] = '"'; break;
            case '\\': dest[j++] = '\\'; dest[j++] = '\\'; break;
            case '\n': dest[j++] = '\\'; dest[j++] = 'n'; break;
            case '\r': dest[j++] = '\\'; dest[j++] = 'r'; break;
            case '\t': dest[j++] = '\\'; dest[j++] = 't'; break;
            default: dest[j++] = src[i]; break;
        }
        i++;
    }
    dest[j] = '\0';
}

int json_parse_attack_config(const char *json, AttackConfig *config) {
    // Default values
    memset(config, 0, sizeof(AttackConfig));
    config->threads = 100;
    config->duration = 60;
    config->method = 7;
    
    // Parse target
    char *target_ptr = strstr(json, "\"target\"");
    if (target_ptr) {
        target_ptr = strchr(target_ptr, ':');
        if (target_ptr) {
            target_ptr++;
            while (*target_ptr == ' ' || *target_ptr == '"') target_ptr++;
            int i = 0;
            while (*target_ptr && *target_ptr != '"' && *target_ptr != ',' && i < 255) {
                config->target[i++] = *target_ptr++;
            }
            config->target[i] = '\0';
        }
    }
    
    // Parse port
    char *port_ptr = strstr(json, "\"port\"");
    if (port_ptr) {
        port_ptr = strchr(port_ptr, ':');
        if (port_ptr) {
            config->port = atoi(port_ptr + 1);
        }
    }
    
    // Parse threads
    char *threads_ptr = strstr(json, "\"threads\"");
    if (threads_ptr) {
        threads_ptr = strchr(threads_ptr, ':');
        if (threads_ptr) {
            config->threads = atoi(threads_ptr + 1);
            if (config->threads > MAX_THREADS) config->threads = MAX_THREADS;
            if (config->threads < 1) config->threads = 1;
        }
    }
    
    // Parse duration
    char *duration_ptr = strstr(json, "\"duration\"");
    if (duration_ptr) {
        duration_ptr = strchr(duration_ptr, ':');
        if (duration_ptr) {
            config->duration = atoi(duration_ptr + 1);
            if (config->duration > 3600) config->duration = 3600;
            if (config->duration < 1) config->duration = 1;
        }
    }
    
    // Parse method
    char *method_ptr = strstr(json, "\"method\"");
    if (method_ptr) {
        method_ptr = strchr(method_ptr, ':');
        if (method_ptr) {
            config->method = atoi(method_ptr + 1);
            if (config->method < 0 || config->method > 7) config->method = 7;
        }
    }
    
    return (strlen(config->target) > 0 && config->port > 0);
}

void json_build_status(char *buffer, size_t size) {
    int active = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (attack_threads[i]) active++;
    }
    
    time_t elapsed = attack_running ? (time(NULL) - attack_start_time) : 0;
    
    snprintf(buffer, size,
        "{"
        "\"running\":%d,"
        "\"active_threads\":%d,"
        "\"target\":\"%s\","
        "\"port\":%d,"
        "\"method\":%d,"
        "\"elapsed\":%ld,"
        "\"duration\":%d,"
        "\"max_threads\":%d"
        "}",
        attack_running, active,
        current_attack.target, current_attack.port,
        current_attack.method, elapsed, current_attack.duration,
        MAX_THREADS
    );
}

// Route handler: find and execute
RouteHandler find_route(const char *path, HttpMethod method) {
    for (int i = 0; route_table[i].handler != NULL; i++) {
        if (strcmp(route_table[i].path, path) == 0 && 
            route_table[i].method == method) {
            return route_table[i].handler;
        }
    }
    return handle_not_found;
}

// Initialize API (called from server.c)
void api_init(void) {
    printf("[API] El Cienco API v%s initialized\n", API_VERSION);
    printf("[API] Route table loaded with %d endpoints\n", 
           (int)(sizeof(route_table) / sizeof(Route)) - 1);
}

// Handle individual API connection
void api_handle_connection(int client_fd) {
    char buffer[API_BUFFER_SIZE] = {0};
    char response[API_BUFFER_SIZE] = {0};
    ApiRequest req;
    ApiResponse res;
    
    // Read request
    ssize_t bytes_read = recv(client_fd, buffer, API_BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    buffer[bytes_read] = '\0';
    
    // Parse request
    parse_http_request(buffer, &req);
    
    // Initialize response
    memset(&res, 0, sizeof(ApiResponse));
    strcpy(res.content_type, "application/json");
    
    // Find and execute route handler
    RouteHandler handler = find_route(
        req.endpoint == ENDPOINT_ROOT ? "/" :
        req.endpoint == ENDPOINT_ATTACK ? "/attack" :
        req.endpoint == ENDPOINT_STOP ? "/stop" :
        req.endpoint == ENDPOINT_STATUS ? "/status" :
        req.endpoint == ENDPOINT_METHODS ? "/methods" :
        req.endpoint == ENDPOINT_CONFIG ? "/config" : "",
        req.method
    );
    
    handler(&req, &res);
    
    // Build and send response
    size_t response_size;
    build_http_response(&res, response, &response_size);
    send(client_fd, response, response_size, 0);
    
    close(client_fd);
}
