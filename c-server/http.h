#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t length;
} HttpResponse;

typedef struct HttpContext HttpContext;
typedef struct HttpRequest HttpRequest;
typedef struct HttpRouter HttpRouter;

typedef int (*RouteHandler)(HttpContext *http, const HttpRequest *req);

struct HttpRequest {
    char method[8];
    char path[256];
};

struct HttpRouter {
    int (*add_route)(HttpRouter *router, const char *path, RouteHandler handler);
    RouteHandler (*match)(HttpRouter *router, const char *path);
    size_t count;
    struct HttpRoute {
        char path[256];
        RouteHandler handler;
    } routes[8];
};

struct HttpContext {
    HttpResponse response;
    void (*destroy)(HttpContext *ctx);
    int (*create_response)(HttpContext *ctx, int status, const char *content_type, const char *body);
    int (*read_request)(HttpContext *ctx, int fd, HttpRequest *out);
};

HttpContext create_http_context(void);
HttpRouter create_http_router(void);

#endif
