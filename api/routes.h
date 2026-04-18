#ifndef ROUTES_H
#define ROUTES_H

#include "../include/api.h"

// Route table
static Route route_table[] = {
    {"/attack",  HTTP_METHOD_POST,   handle_attack},
    {"/stop",    HTTP_METHOD_POST,   handle_stop},
    {"/status",  HTTP_METHOD_GET,    handle_status},
    {"/methods", HTTP_METHOD_GET,    handle_methods},
    {"/config",  HTTP_METHOD_GET,    handle_config},
    {"/config",  HTTP_METHOD_POST,   handle_config},
    {"",         HTTP_METHOD_UNKNOWN, NULL}  // Sentinel
};

// Find route handler
static inline RouteHandler find_route(const char *path, HttpMethod method) {
    for (int i = 0; route_table[i].handler != NULL; i++) {
        if (strcmp(route_table[i].path, path) == 0 && 
            route_table[i].method == method) {
            return route_table[i].handler;
        }
    }
    return handle_not_found;
}

#endif // ROUTES_H
