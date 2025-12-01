#include "http.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum { HTTP_READ_BUFFER_SIZE = 2048 };
enum { HTTP_MAX_ROUTES = 8 };

static int http_read_request(HttpContext *ctx, int fd, HttpRequest *out);

static const char *reason_phrase(int status) {
    switch (status) {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    default:
        return "OK";
    }
}

static char *build_response(int status, const char *content_type, const char *body, size_t body_len,
                            size_t *out_len) {
    const char *template =
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n";

    const char *reason = reason_phrase(status);
    size_t header_len = (size_t)snprintf(NULL, 0, template, status, reason, content_type, body_len);
    char *buffer = malloc(header_len + body_len + 1);
    if (buffer == NULL) {
        return NULL;
    }

    int written = snprintf(buffer, header_len + 1, template, status, reason, content_type, body_len);
    if (written < 0) {
        free(buffer);
        return NULL;
    }

    memcpy(buffer + (size_t)written, body, body_len);
    buffer[header_len + body_len] = '\0';
    *out_len = (size_t)written + body_len;

    return buffer;
}

static void http_response_destroy(HttpContext *ctx) {
    free(ctx->response.data);
    ctx->response.data = NULL;
    ctx->response.length = 0;
}

static int http_response_create(HttpContext *ctx, int status, const char *content_type,
                                const char *body) {
    if (body == NULL || content_type == NULL) {
        return -1;
    }

    http_response_destroy(ctx);
    size_t response_len = 0;
    char *built = build_response(status, content_type, body, strlen(body), &response_len);
    if (built == NULL) {
        return -1;
    }

    ctx->response.data = built;
    ctx->response.length = response_len;
    return 0;
}


static void destroy_http_context(HttpContext *ctx) {
    if (ctx == NULL) {
        return;
    }

   http_response_destroy(ctx);
}

HttpContext create_http_context(void) {
    HttpContext ctx = {
        .response = {.data = NULL, .length = 0},
        .destroy = destroy_http_context,
        .create_response = http_response_create,
        .read_request = http_read_request,
    };

    return ctx;
}

static int http_router_add(HttpRouter *router, const char *path, RouteHandler handler) {
    if (router == NULL || path == NULL || handler == NULL) {
        return -1;
    }
    if (router->count >= HTTP_MAX_ROUTES) {
        return -1;
    }

    size_t path_len = strlen(path);
    if (path_len == 0 || path_len >= sizeof(router->routes[0].path)) {
        return -1;
    }

    memcpy(router->routes[router->count].path, path, path_len + 1);
    router->routes[router->count].handler = handler;
    router->count++;
    return 0;
}

static RouteHandler http_router_match(HttpRouter *router, const char *path) {
    if (router == NULL || path == NULL) {
        return NULL;
    }

    size_t i = 0;
    while (i < router->count) {
        if (strcmp(path, router->routes[i].path) == 0) {
            return router->routes[i].handler;
        }
        i++;
    }
    return NULL;
}

HttpRouter create_http_router(void) {
    HttpRouter router = {
        .add_route = http_router_add,
        .match = http_router_match,
    };

    return router;
}

static int http_read_request(HttpContext *ctx, int fd, HttpRequest *out) {
    if (out == NULL) {
        return -1;
    }

    char buffer[HTTP_READ_BUFFER_SIZE];
    ssize_t received = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        return -1;
    }
    buffer[received] = '\0';

    char *method_end = strchr(buffer, ' ');
    if (method_end == NULL) {
        return -1;
    }
    size_t method_len = (size_t)(method_end - buffer);
    if (method_len == 0 || method_len >= sizeof(out->method)) {
        return -1;
    }

    char *path_start = method_end + 1;
    char *path_end = strchr(path_start, ' ');
    if (path_end == NULL) {
        return -1;
    }
    size_t path_len = (size_t)(path_end - path_start);
    if (path_len == 0 || path_len >= sizeof(out->path)) {
        return -1;
    }

    memcpy(out->method, buffer, method_len);
    out->method[method_len] = '\0';
    memcpy(out->path, path_start, path_len);
    out->path[path_len] = '\0';

    // Best-effort drain remaining data without blocking.
    if (strstr(buffer, "\r\n\r\n") == NULL) {
        while ((received = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT)) > 0) {
            if (received < (ssize_t)sizeof(buffer)) {
                break;
            }
        }
    }

    (void)ctx;
    return 0;
}
