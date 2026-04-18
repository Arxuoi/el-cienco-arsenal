#ifndef API_H
#define API_H

#include "elcienco.h"

// API version
#define API_VERSION "2.3.1"
#define API_MAX_CLIENTS 10
#define API_BUFFER_SIZE 8192

// HTTP status codes
#define HTTP_OK                   200
#define HTTP_BAD_REQUEST         400
#define HTTP_NOT_FOUND           404
#define HTTP_METHOD_NOT_ALLOWED  405
#define HTTP_INTERNAL_ERROR      500

// API endpoints
typedef enum {
    ENDPOINT_ROOT,
    ENDPOINT_ATTACK,
    ENDPOINT_STOP,
    ENDPOINT_STATUS,
    ENDPOINT_METHODS,
    ENDPOINT_CONFIG,
    ENDPOINT_UNKNOWN
} ApiEndpoint;

// HTTP methods
typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_UNKNOWN
} HttpMethod;

// API request structure
typedef struct {
    HttpMethod method;
    ApiEndpoint endpoint;
    char body[API_BUFFER_SIZE];
    int content_length;
    char headers[10][256];
    int header_count;
} ApiRequest;

// API response structure
typedef struct {
    int status_code;
    char content_type[64];
    char body[API_BUFFER_SIZE];
} ApiResponse;

// Route handler function type
typedef void (*RouteHandler)(ApiRequest *req, ApiResponse *res);

// Route entry
typedef struct {
    char path[64];
    HttpMethod method;
    RouteHandler handler;
} Route;

// API functions
void api_init(void);
void api_handle_connection(int client_fd);
ApiEndpoint parse_endpoint(const char *path);
HttpMethod parse_http_method(const char *method);
void parse_http_request(const char *raw_request, ApiRequest *req);
void build_http_response(ApiResponse *res, char *buffer, size_t *size);

// Route handlers
void handle_attack(ApiRequest *req, ApiResponse *res);
void handle_stop(ApiRequest *req, ApiResponse *res);
void handle_status(ApiRequest *req, ApiResponse *res);
void handle_methods(ApiRequest *req, ApiResponse *res);
void handle_config(ApiRequest *req, ApiResponse *res);
void handle_not_found(ApiRequest *req, ApiResponse *res);

// JSON utilities
void json_escape(char *dest, const char *src, size_t max_len);
int json_parse_attack_config(const char *json, AttackConfig *config);
void json_build_status(char *buffer, size_t size);

#endif // API_H
