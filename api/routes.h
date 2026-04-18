#ifndef ROUTES_H
#define ROUTES_H

#include "../include/api.h"

// ============================================
// ROUTE TABLE DECLARATION
// ============================================

// Global route table - defined in routes.c
extern Route route_table[];

// ============================================
// ROUTE HANDLER FUNCTION DECLARATIONS
// ============================================

// Core attack handlers
void handle_attack(ApiRequest *req, ApiResponse *res);
void handle_stop(ApiRequest *req, ApiResponse *res);
void handle_status(ApiRequest *req, ApiResponse *res);

// Info handlers
void handle_methods(ApiRequest *req, ApiResponse *res);
void handle_config(ApiRequest *req, ApiResponse *res);

// Fallback handler
void handle_not_found(ApiRequest *req, ApiResponse *res);

// ============================================
// ROUTE UTILITIES
// ============================================

// Find route handler by path and method
// Returns: Function pointer to handler, or handle_not_found if not found
RouteHandler find_route(const char *path, HttpMethod method);

// ============================================
// ROUTE TABLE SIZE
// ============================================

// Number of routes in the table (excluding sentinel)
#define ROUTE_COUNT (sizeof(route_table) / sizeof(Route) - 1)

#endif // ROUTES_H
