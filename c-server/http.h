#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t length;
} HttpResponse;

typedef struct HttpContext HttpContext;
struct HttpContext {
    HttpResponse response;
    void (*destroy)(HttpContext *ctx);
    int (*create_response)(HttpContext *ctx, const char *body);
};

HttpContext create_http_context(void);

#endif
