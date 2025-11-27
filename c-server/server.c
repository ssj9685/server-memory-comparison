#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int signum) {
    (void)signum;
    keep_running = 0;
}

static void install_signal_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handle_signal;
    // Avoid SA_RESTART so that blocking syscalls (accept) return EINTR and
    // the loop can observe keep_running and exit on SIGTERM/SIGINT.
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("sigaction SIGINT");
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("sigaction SIGTERM");
    }

    struct sigaction ignore_pipe;
    memset(&ignore_pipe, 0, sizeof(ignore_pipe));
    sigemptyset(&ignore_pipe.sa_mask);
    ignore_pipe.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &ignore_pipe, NULL) != 0) {
        perror("sigaction SIGPIPE");
    }
}

static int parse_port(const char *value, int fallback) {
    if (value == NULL || *value == '\0') {
        return fallback;
    }
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (end == value || parsed <= 0 || parsed > 65535) {
        return fallback;
    }
    return (int)parsed;
}

static char *build_response(const char *body, size_t body_len, size_t *out_len) {
    const char *template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
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

int main(void) {
    install_signal_handlers();

    const char *body_env = getenv("RESPONSE_BODY");
    const char *body = (body_env != NULL) ? body_env : "Hello from C server\n";
    size_t body_len = strlen(body);

    size_t response_len = 0;
    char *response = build_response(body, body_len, &response_len);
    if (response == NULL) {
        fprintf(stderr, "Failed to build HTTP response buffer\n");
        return EXIT_FAILURE;
    }

    int port = parse_port(getenv("PORT"), 8080);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        free(response);
        return EXIT_FAILURE;
    }

    int reuse = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        free(response);
        return EXIT_FAILURE;
    }

    if (listen(listen_fd, 128) < 0) {
        perror("listen");
        close(listen_fd);
        free(response);
        return EXIT_FAILURE;
    }

    printf("C server listening on http://127.0.0.1:%d\n", port);
    fflush(stdout);

    char buffer[2048];
    while (keep_running) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            break;
        }

        // Read and discard the request to avoid leaving unread data on the socket.
        (void)recv(client_fd, buffer, sizeof(buffer), 0);

        size_t sent_total = 0;
        while (sent_total < response_len) {
            ssize_t sent = send(client_fd, response + sent_total, response_len - sent_total, 0);
            if (sent <= 0) {
                break;
            }
            sent_total += (size_t)sent;
        }
        close(client_fd);
    }

    close(listen_fd);
    free(response);
    return EXIT_SUCCESS;
}
