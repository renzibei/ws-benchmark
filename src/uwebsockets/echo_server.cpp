// This file is modified from uwebsockets example
/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include "test_def.h"

/* This is a simple WebSocket echo server example.
 * You may compile it with "WITH_OPENSSL=1 make" or with "make" */

int main(int argc, const char** argv) {

    if (argc < 5) {
        printf("Invalid parameters!\nUsage: ./echo_server ip_address port max_msg_len export_filename\n");
        return -1;
    }

    const char* SERVER_IP = argv[1];

    int SERVER_PORT = atoi(argv[2]);
    if (SERVER_PORT <= 0) {
        printf("Invalid port: %s\n", argv[2]);
        return -1;
    }
    long long max_msg_len = atoll(argv[3]);
    if (max_msg_len <= 0) {
        printf("invalid max_msg_len: %s\n", argv[3]);
        return -1;
    }
    size_t MAX_DATA_LEN = size_t(max_msg_len);

    const char* export_file_path = argv[4];
    FILE *output_fp = fopen(export_file_path, "w");
    if (output_fp == nullptr) {
        printf("Cannot create file at %s\n", export_file_path);
        return -1;
    }
    printf("Will output to %s\n", export_file_path);
    // msg_size, rx goodput, tx goodput, rx mm mps, tx mm mps
    fprintf(output_fp, "msg_size,rx_goodput,tx_goodput,rx_mm_mps,tx_mm_mps,connection_cnt\n");

    uint64_t last_interval_send_bytes = 0;
    uint64_t last_interval_recv_bytes = 0;
    uint64_t last_interval_recv_msg_cnt = 0;
    uint64_t last_interval_send_msg_cnt = 0;
    size_t last_msg_size = 0;
    int64_t interval_start_ns = 0;
    int64_t active_connections = 0;

    /* ws->getUserData returns one of these */
    struct PerSocketData {

        /* Fill with user data */
    };

    auto count_stats = [&](){
        if (((last_interval_recv_msg_cnt & 0xfffUL)
                           && (last_interval_recv_bytes <= MAX_DATA_LEN * 4096UL)
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
        printf("last_msg_size: %zu, avg rx+tx goodput: %.2lf Mbps, %.4lf 10^6 msg/sec,"
               " active conn: %ld\n",
               last_msg_size,
               recv_throughput_mbps + send_throughput_mbps,
               recv_mm_msg_per_sec + send_mm_msg_per_sec,
               active_connections);
        if (output_fp != nullptr) {
            // msg_size, rx goodput, tx goodput, rx mm mps, tx mm mps, conn cnt
            fprintf(output_fp, "%zu,%.3lf,%.3lf,%lf,%lf,%zu\n",
                    last_msg_size,
                    recv_throughput_mbps,
                    send_throughput_mbps,
                    recv_mm_msg_per_sec,
                    send_mm_msg_per_sec,
                    active_connections
            );
            fflush(output_fp);
        }

        last_interval_recv_bytes = 0;
        last_interval_send_bytes = 0;
        last_interval_recv_msg_cnt = 0;
        last_interval_send_msg_cnt = 0;
        interval_start_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    };

    printf("Prepare to run\n");

    /* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support.
     * You may swap to using uWS:App() if you don't need SSL */
    uWS::App(
//            {
            /* There are example certificates in uWebSockets.js repo */
//            .key_file_name = "misc/key.pem",
//            .cert_file_name = "misc/cert.pem",
//            .passphrase = "1234"
//        }
        ).ws<PerSocketData>("/", {
            /* Settings */
//            .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
            .compression = uWS::CompressOptions(uWS::DISABLED),
            .maxPayloadLength = (uint32_t)MAX_DATA_LEN,
            .idleTimeout = 16,
            .maxBackpressure = 100 * 1024 * 1024,
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend = false,
            .sendPingsAutomatically = true,
            /* Handlers */
            .upgrade = nullptr,
            .open = [&](auto */*ws*/) {
                ++active_connections;
                /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
//                std::cout << "connection opened\n";
            },
            .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
                size_t msg_size = message.size();
                if (interval_start_ns == 0) {
                    interval_start_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                }
                last_interval_recv_bytes += msg_size;
                ++last_interval_recv_msg_cnt;
                ws->send(message, opCode, true);
                // please note that send does not mean all sent to the wire
                last_msg_size = msg_size;
                last_interval_send_bytes += msg_size;
                ++last_interval_send_msg_cnt;
                count_stats();
            },
            .drain = [](auto */*ws*/) {
                /* Check ws->getBufferedAmount() here */
            },
            .ping = [](auto */*ws*/, std::string_view) {
                /* Not implemented yet */
            },
            .pong = [](auto */*ws*/, std::string_view) {
                /* Not implemented yet */
            },
            .close = [&](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
                --active_connections;
                /* You may access ws->getUserData() here */
//                std::cout << "connection closed\n";
            }
    }).listen(SERVER_IP, SERVER_PORT, [&](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << SERVER_PORT << std::endl;

        }
    }).run();
}
