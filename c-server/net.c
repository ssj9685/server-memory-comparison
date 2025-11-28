#include "net.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum { READ_BUFFER_SIZE = 2048 };

static void listener_close(NetContext *ctx) {
    Listener *listener = &ctx -> listener;

    if (listener == NULL) {
        return;
    }
    if (listener->fd >= 0) {
        close(listener->fd);
    }
    listener->fd = -1;
}

static int listener_open(NetContext *ctx) {
    ctx->listener.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->listener.fd < 0) {
        perror("socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(ctx->listener.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)ctx->config.port);

    if (bind(ctx->listener.fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        listener_close(ctx);
        return -1;
    }

    if (listen(ctx->listener.fd, ctx->config.backlog) < 0) {
        perror("listen");
        listener_close(ctx);
        return -1;
    }

    return 0;
}

static int send_all(int fd, const char *data, size_t length) {
    size_t sent_total = 0;

    while (sent_total < length) {
        ssize_t sent = send(fd, data + sent_total, length - sent_total, 0);
        if (sent <= 0) {
            return -1;
        }
        sent_total += (size_t)sent;
    }

    return 0;
}

static void discard_request(int fd) {
    char buffer[READ_BUFFER_SIZE];
    (void)recv(fd, buffer, sizeof(buffer), 0);
}

NetContext create_net_context(ServerConfig config) {
    NetContext ctx = {
        .listener = {.fd = -1},
        .config = config,
        .open = listener_open,
        .close = listener_close,
        .send_all = send_all,
        .discard_request = discard_request,
    };
    return ctx;
}
