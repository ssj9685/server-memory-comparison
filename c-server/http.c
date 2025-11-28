#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *build_response(const char *body, size_t body_len, size_t *out_len) {
    const char *template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n";

    size_t header_len = (size_t)snprintf(NULL, 0, template, body_len);
    char *buffer = malloc(header_len + body_len + 1);
    if (buffer == NULL) {
        return NULL;
    }

    int written = snprintf(buffer, header_len + 1, template, body_len);
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

static int http_response_create(HttpContext *ctx, const char *body) {
    if (body == NULL) {
        return -1;
    }

    http_response_destroy(ctx);
    size_t response_len = 0;
    char *built = build_response(body, strlen(body), &response_len);
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
    };

    return ctx;
}
