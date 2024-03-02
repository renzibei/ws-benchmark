#include <libusockets.h>
const int SSL = 1;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define REAL_HOST "www.google.com"
#define REAL_HOST "10.5.96.3"

const char* host = REAL_HOST;
const char request[] = "GET / HTTP/1.1\r\nHost: " REAL_HOST "\r\n\r\n";

//int port;
//int connections;

//int responses;

struct ClientContext {
    int has_written_bytes{0};
    int received_bytes{0};
};

/* We don't need any of these */
void on_wakeup(struct us_loop_t *loop) {

}

void on_pre(struct us_loop_t *loop) {

}

/* This is not HTTP POST, it is merely an event emitted post loop iteration */
void on_post(struct us_loop_t *loop) {

}

us_socket_t * try_request(us_socket_t *s) {
    ClientContext *client_context = (ClientContext *) us_socket_ext(SSL, s);
    int has_written = client_context->has_written_bytes;
    int target_written = sizeof(request) - 1;
    if (has_written < target_written) {
        int written = us_socket_write(SSL, s, request + has_written, target_written - has_written, 0);
        client_context->has_written_bytes += written;
    }
    printf("Write %d length request to server\n", client_context->has_written_bytes);
    return s;
}

struct us_socket_t *on_http_socket_writable(struct us_socket_t *s) {
    try_request(s);
    return s;
}



struct us_socket_t *on_http_socket_close(struct us_socket_t *s, int code, void *reason) {
    printf("HTTPS connection closed, code:%d\n", code);
    return s;
}

struct us_socket_t *on_http_socket_end(struct us_socket_t *s) {
    printf("HTTPS connection end\n");
    return us_socket_close(SSL, s, 0, NULL);
}

struct us_socket_t *on_http_socket_data(struct us_socket_t *s, char *data, int length) {

//    us_socket_write(SSL, s, request, sizeof(request) - 1, 0);
    ClientContext *client_context = (ClientContext *) us_socket_ext(SSL, s);
    client_context->received_bytes += length;
    printf("Receive data from server, length: %d\n", length);
    if (length > 3) {
        printf("Char1: %c, Char2: %c, Char3: %c, Char4: %c\n", data[0], data[1], data[2], data[3]);
    }
    fwrite(data, 1, length, stdout);
    printf("\n");
//    responses++;

    return s;
}

struct us_socket_t *on_http_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    printf("Connect on open\n");
    ClientContext *client_context = (ClientContext *) us_socket_ext(SSL, s);
    client_context->has_written_bytes = 0;
    client_context->received_bytes = 0;
    return try_request(s);
    /* Send a request */
//    us_socket_write(SSL, s, request, sizeof(request) - 1, 0);

//    if (--connections) {
//        us_socket_context_connect(SSL, us_socket_context(SSL, s), host, port, NULL, 0, 0);
//    } else {
//        printf("Running benchmark now...\n");

//        us_socket_timeout(SSL, s, LIBUS_TIMEOUT_GRANULARITY);
//        us_socket_long_timeout(SSL, s, 1);
//    }

//    return s;
}

struct us_socket_t *on_http_socket_long_timeout(struct us_socket_t *s) {
    /* Print current statistics */
    printf("--- Minute mark ---\n");

    us_socket_long_timeout(SSL, s, 1);

    return s;
}

struct us_socket_t *on_http_socket_timeout(struct us_socket_t *s) {
    /* Print current statistics */
//    printf("Req/sec: %f\n", ((float)responses) / LIBUS_TIMEOUT_GRANULARITY);
    printf("On Timeout\n");
//    responses = 0;
    us_socket_timeout(SSL, s, LIBUS_TIMEOUT_GRANULARITY);

    return s;
}

struct us_socket_t *on_http_socket_connect_error(struct us_socket_t *s, int code) {
    printf("Cannot connect to server\n");

    return s;
}

int main(int argc, char **argv) {

    /* Parse host and port */
//    if (argc != 4) {
//        printf("Usage: connections host port\n");
//        return 0;
//    }
//    const int port = 443;
    const int port = 58600;
//    port = atoi(argv[3]);
//    host = (char*)malloc(strlen(argv[2]) + 1);
//    const char* host = "www.google.com";

//    memcpy(host, argv[2], strlen(argv[2]) + 1);
//    connections = atoi(argv[1]);

    /* Create the event loop */
    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

    /* Create a socket context for HTTP */
    struct us_socket_context_options_t options = {
//            .ca_file_name = "./google.pem",
//        .ca_file_name = "/etc/ssl/certs/ca-certificates.crt",
    };
    struct us_socket_context_t *client_context = us_create_socket_context(SSL, loop, sizeof(ClientContext), options);

    if (!client_context) {
        printf("Could not load SSL cert/key\n");
        exit(0);
    }

    /* Set up event handlers */
    us_socket_context_on_open(SSL, client_context, on_http_socket_open);
    us_socket_context_on_data(SSL, client_context, on_http_socket_data);
    us_socket_context_on_writable(SSL, client_context, on_http_socket_writable);
    us_socket_context_on_close(SSL, client_context, on_http_socket_close);
    us_socket_context_on_timeout(SSL, client_context, on_http_socket_timeout);
    us_socket_context_on_long_timeout(SSL, client_context, on_http_socket_long_timeout);
    us_socket_context_on_end(SSL, client_context, on_http_socket_end);
    us_socket_context_on_connect_error(SSL, client_context, on_http_socket_connect_error);

    /* Start making HTTP connections */
    if (!us_socket_context_connect(SSL, client_context, host, port, NULL, 0, sizeof(ClientContext))) {
        printf("Cannot connect to server\n");
    }

    us_loop_run(loop);

    us_socket_context_free(SSL, client_context);
    us_loop_free(loop);
}
