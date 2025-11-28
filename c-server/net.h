#ifndef NET_H
#define NET_H

#include <stddef.h>

typedef struct {
    int port;
    int backlog;
} ServerConfig;

typedef struct {
    int fd;
} Listener;

typedef struct NetContext NetContext;
struct NetContext{
    Listener listener;
    ServerConfig config;
    int (*open)(NetContext *ctx);
    void (*close)(NetContext *ctx);
    int (*send_all)(int fd, const char *data, size_t length);
    void (*discard_request)(int fd);
};

NetContext create_net_context(ServerConfig config);

#endif
