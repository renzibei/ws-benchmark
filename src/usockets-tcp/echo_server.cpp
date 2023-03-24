/* This is a basic TCP/TLS echo server. */

#include <libusockets.h>
#include <cerrno>
const int SSL = 0;

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <chrono>
#include "test_def.h"

struct Buffer {
    char data[MAX_DATA_LEN];
    size_t start_pos;
    size_t size;
};

/* Our socket extension */
struct echo_socket {
//    char *backpressure;
//    int length;
//    std::vector<Buffer> data_vec;
    Buffer buf;
};

/* Our socket context extension */
struct echo_context {

};

/* Loop wakeup handler */
void on_wakeup(struct us_loop_t *loop) {

}

/* Loop pre iteration handler */
void on_pre(struct us_loop_t *loop) {

}

/* Loop post iteration handler */
void on_post(struct us_loop_t *loop) {

}

uint64_t last_interval_send_bytes = 0;
uint64_t last_interval_recv_bytes = 0;
uint64_t last_interval_recv_msg_cnt = 0;
uint64_t last_interval_send_msg_cnt = 0;

int64_t interval_start_ns = 0;
int64_t active_connections = 0;

void CountStats() {
    if (((last_interval_recv_msg_cnt & 0xffffUL)
//                && (last_interval_recv_bytes <= MAX_DATA_LEN * 4096UL)
                  ) || last_interval_recv_msg_cnt == 0) {
        return;
    }
    int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    int64_t interval_ns = now_ns - interval_start_ns;
    constexpr int64_t BITS_PER_BYTE = 8;
    double interval_sec = double(interval_ns) / (1e+9);
    double recv_throughput_mbps = double((last_interval_recv_bytes) *
                                         BITS_PER_BYTE) / (1e+6) / interval_sec;
    double send_throughput_mbps = double((last_interval_send_bytes) *
                                         BITS_PER_BYTE) / (1e+6) / interval_sec;

    double recv_mm_msg_per_sec = double(last_interval_recv_msg_cnt)
                                 / (1e+6) / interval_sec;
    double send_mm_msg_per_sec = double(last_interval_send_msg_cnt)
                                 / (1e+6) / interval_sec;
//            if (last_interval_send_msg_cnt != last_interval_recv_msg_cnt) {
//                printf("Warning, last interval recv: %zu msg, send %zu msg\n",
//                       last_interval_recv_msg_cnt, last_interval_send_msg_cnt);
//            }
    printf("avg rx+tx goodput: %.2lf Mbps, %.4lf 10^6 msg/sec,"
           "active_client_cnt: %ld\n",
           recv_throughput_mbps + send_throughput_mbps,
           recv_mm_msg_per_sec + send_mm_msg_per_sec,
           active_connections);

    last_interval_recv_bytes = 0;
    last_interval_send_bytes = 0;
    last_interval_recv_msg_cnt = 0;
    last_interval_send_msg_cnt = 0;
    interval_start_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

}

/* Socket writable handler */
struct us_socket_t *on_echo_socket_writable(struct us_socket_t *s) {
    struct echo_socket *es = (struct echo_socket *) us_socket_ext(SSL, s);

    /* Continue writing out our backpressure */
    int written = us_socket_write(SSL, s, es->buf.data + es->buf.start_pos, es->buf.size, 0);
    es->buf.size -= written;
    es->buf.start_pos += written;
    last_interval_send_bytes += written;
    if (es->buf.size == 0) {
        ++last_interval_send_msg_cnt;
        es->buf.start_pos = 0;
        CountStats();
    }

//    if (written != es->buf.size) {
//        char *new_buffer = (char *) malloc(es->length - written);
//        memcpy(new_buffer, es->backpressure, es->length - written);
//        free(es->backpressure);
//        es->backpressure = new_buffer;
//        es->length -= written;
//    } else {
//        free(es->backpressure);
//        es->length = 0;
//    }

    /* Client is not boring */
//    us_socket_timeout(SSL, s, 30);

    return s;
}

/* Socket closed handler */
struct us_socket_t *on_echo_socket_close(struct us_socket_t *s, int code, void *reason) {
    struct echo_socket *es = (struct echo_socket *) us_socket_ext(SSL, s);
    --active_connections;
//    printf("Client disconnected\n");
//    free(es->buf.data);
//    free(es->backpressure);

    return s;
}

/* Socket half-closed handler */
struct us_socket_t *on_echo_socket_end(struct us_socket_t *s) {
    us_socket_shutdown(SSL, s);
    return us_socket_close(SSL, s, 0, NULL);
}

/* Socket data handler */
struct us_socket_t *on_echo_socket_data(struct us_socket_t *s, char *data, int length) {
    struct echo_socket *es = (struct echo_socket *) us_socket_ext(SSL, s);
    last_interval_recv_bytes += length;
    if (length > 0) {
        ++last_interval_recv_msg_cnt;
    }
    /* Print the data we received */
//    printf("Client sent <%.*s>\n", length, data);
    if (length == MAX_DATA_LEN) {
        int written = us_socket_write(SSL, s, data, length, 0);
        last_interval_send_bytes += written;
        if (written != length) {
            memcpy(es->buf.data, data + written, length - written);
            es->buf.size = length - written;
        }
        else {
            ++last_interval_send_msg_cnt;

            CountStats();
        }

    }
    else {
        memcpy(es->buf.data + es->buf.size, data, length);
        es->buf.size += length;
        if (es->buf.size == MAX_DATA_LEN) {
            int written = us_socket_write(SSL, s, es->buf.data, es->buf.size, 0);
            es->buf.start_pos += written;
            last_interval_send_bytes += written;
            es->buf.size -= written;
            if (es->buf.size == 0) {
                ++last_interval_send_msg_cnt;
                es->buf.start_pos = 0;
                CountStats();
            }
        }

    }
    /* Send it back or buffer it up */



    /* Client is not boring */
//    us_socket_timeout(SSL, s, 30);

    return s;
}

/* Socket opened handler */
struct us_socket_t *on_echo_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    struct echo_socket *es = (struct echo_socket *) us_socket_ext(SSL, s);

    /* Initialize the new socket's extension */
//    es->backpressure = 0;
//    es->length = 0;
//    es->buf.data = (char*)malloc(MAX_DATA_LEN);
    es->buf.start_pos = 0;
    es->buf.size = 0;
    ++active_connections;
    /* Start a timeout to close the socket if boring */
//    us_socket_timeout(SSL, s, 30);

//    printf("Client connected\n");

    return s;
}

/* Socket timeout handler */
struct us_socket_t *on_echo_socket_timeout(struct us_socket_t *s) {
    printf("Client was idle for too long\n");
    return us_socket_close(SSL, s, 0, NULL);
}

int main() {
    /* The event loop */
    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

    /* Socket context */
    struct us_socket_context_options_t options = {};
//    options.key_file_name = "/home/alexhultman/uWebSockets.js/misc/key.pem";
//    options.cert_file_name = "/home/alexhultman/uWebSockets.js/misc/cert.pem";
//    options.passphrase = "1234";

    struct us_socket_context_t *echo_context = us_create_socket_context(SSL, loop, sizeof(struct echo_context), options);


    /* Registering event handlers */
    us_socket_context_on_open(SSL, echo_context, on_echo_socket_open);
    us_socket_context_on_data(SSL, echo_context, on_echo_socket_data);
    us_socket_context_on_writable(SSL, echo_context, on_echo_socket_writable);
    us_socket_context_on_close(SSL, echo_context, on_echo_socket_close);
    us_socket_context_on_timeout(SSL, echo_context, on_echo_socket_timeout);
    us_socket_context_on_end(SSL, echo_context, on_echo_socket_end);

    /* Start accepting echo sockets */
    struct us_listen_socket_t *listen_socket = us_socket_context_listen(SSL, echo_context,
            test::SERVER_IP, SERVER_PORT, 0, sizeof(struct echo_socket));

    if (listen_socket) {
        printf("Listening on port %d...\n", SERVER_PORT);
        us_loop_run(loop);
    } else {
        printf("Failed to listen! %s\n",
               std::strerror(errno));
    }
}
