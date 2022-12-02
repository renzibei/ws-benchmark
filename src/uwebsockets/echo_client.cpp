/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include "test_def.h"
#include <chrono>
#include <cstdio>

// client of uwebsockets is not usable now! Use ws clients from other libraries

int main(int argc, const char** argv) {

    if (argc != 4) {
        printf("Invalid parameters!\nUsage: ./echo_server port max_msg_len msg_cnt\n");
        return -1;
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        printf("Invalid port: %s\n", argv[1]);
        return -1;
    }
    long long max_msg_len = atoll(argv[2]);
    if (max_msg_len <= 0) {
        printf("invalid max_msg_len: %s\n", argv[2]);
        return -1;
    }

    int max_msg_cnt = atoi(argv[3]);
    if (max_msg_cnt <= 0) {
        printf("invalid max_msg_cnt: %s\n", argv[3]);
    }

    std::string host_addr = std::string("ws://") + std::string(test::listen_addr) + ":" + std::to_string(port);
    printf("Will connect to %s\n", host_addr.c_str());

    std::string first_msg(max_msg_len, 0);
    for (int i = 0; i < max_msg_len; ++i) {
        first_msg[i] = rand() % 10 + '0';
    }
    /* ws->getUserData returns one of these */
    struct PerSocketData {
        int64_t msg_cnt = 0;
        /* Fill with user data */
        int64_t last_ns = 0;
        int64_t start_ns = 0;
    };



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
            .maxPayloadLength = (uint32_t)max_msg_len,
            .idleTimeout = 16,
            .maxBackpressure = 100 * 1024 * 1024,
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend = false,
            .sendPingsAutomatically = true,
            /* Handlers */
            .upgrade = nullptr,
            .open = [&](auto * ws) {
                /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
                std::cout << "connection opened\n";
                auto* user_data = ws->getUserData();
                user_data->start_ns =  (user_data->last_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count());
                ws->send(first_msg, uWS::OpCode::TEXT, true);
            },
            .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
                auto *user_data = ws->getUserData();
                auto now_msg_cnt = ++user_data->msg_cnt;
                if (now_msg_cnt == max_msg_cnt) {
                    auto now_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    auto pass_ns = now_ns - user_data->start_ns;
                    double rtt_ns = double(pass_ns) / now_msg_cnt;
                    printf("Total avg rtt latency: %.3f us\n", rtt_ns / 1000.0);
                }
                ws->send(message, opCode, true);
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
            .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
                /* You may access ws->getUserData() here */
                std::cout << "connection closed\n";
            }
    }).connect(host_addr, [&](auto *, auto *) {
        printf("connect call\n");
//        if (listen_socket) {
//            std::cout << "Listening on port " << port << std::endl;
//        }
    }).run();
}
