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

static const char DEFAULT_BODY[] = "<script>alert('Hello world')</script>";
enum { PORT = 8080 };

int main(void) {
    SignalOps signals = create_signal_ops();
    NetContext net = create_net_context((ServerConfig){
        .port = PORT,
        .backlog = 128,
    });
    HttpContext http = create_http_context();
    int status = EXIT_FAILURE;

    if (signals.install()) {
        goto cleanup;
    }

    if(http.create_response(&http, DEFAULT_BODY)){
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

        net.discard_request(client_fd);
        (void)net.send_all(client_fd, http.response.data, http.response.length);
        close(client_fd);
    }

    status = EXIT_SUCCESS;

cleanup:
    net.close(&net);
    http.destroy(&http);
    return status;
}
