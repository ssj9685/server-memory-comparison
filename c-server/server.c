#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"
#include "net.h"
#include "signals.h"

enum { PORT = 8080 };

static int handle_root(HttpContext *http, const HttpRequest *req) {
    (void)req;
    return http->create_response(http, 200, "text/html", "<script>alert('Hello world')</script>");
}

static int handle_health(HttpContext *http, const HttpRequest *req) {
    (void)req;
    return http->create_response(http, 200, "text/plain", "ok\n");
}

static int handle_ping(HttpContext *http, const HttpRequest *req) {
    (void)req;
    return http->create_response(http, 200, "text/plain", "pong\n");
}

static int handle_not_found(HttpContext *http, const HttpRequest *req) {
    (void)req;
    return http->create_response(http, 404, "text/plain", "not found\n");
}

static int handle_bad_request(HttpContext *http, const HttpRequest *req) {
    (void)req;
    return http->create_response(http, 400, "text/plain", "bad request\n");
}

static int handle_method_not_allowed(HttpContext *http, const HttpRequest *req) {
    (void)req;
    return http->create_response(http, 405, "text/plain", "method not allowed\n");
}

int main(void) {
    SignalOps signals = create_signal_ops();
    HttpContext http = create_http_context();
    NetContext net = create_net_context((ServerConfig){
        .port = PORT,
        .backlog = 128,
    });
    HttpRouter router = create_http_router();
    int status = EXIT_FAILURE;

    if (signals.install()) {
        goto cleanup;
    }

    if (router.add_route(&router, "/", handle_root) ||
        router.add_route(&router, "/health", handle_health) ||
        router.add_route(&router, "/ping", handle_ping)) {
        goto cleanup;
    }
    
    if (net.open(&net)) {
        goto cleanup;
    }

    printf("C server listening on http://127.0.0.1:%d\n", PORT);
    fflush(stdout);

    while (*(signals.flag)) {
        int client_fd = accept(net.listener.fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            break;
        }

        HttpRequest req;
        if (http.read_request(&http, client_fd, &req)) {
            if (handle_bad_request(&http, NULL) == 0) {
                (void)net.send_all(client_fd, http.response.data, http.response.length);
            }
            close(client_fd);
            continue;
        }

        if (strcmp(req.method, "GET") != 0) {
            if (handle_method_not_allowed(&http, &req) == 0) {
                (void)net.send_all(client_fd, http.response.data, http.response.length);
            }
            close(client_fd);
            continue;
        }

        RouteHandler handler = router.match(&router, req.path);
        if (handler == NULL) {
            handler = handle_not_found;
        }

        if (handler(&http, &req) == 0) {
            (void)net.send_all(client_fd, http.response.data, http.response.length);
        }
        close(client_fd);
    }

    status = EXIT_SUCCESS;

cleanup:
    net.close(&net);
    http.destroy(&http);
    return status;
}
