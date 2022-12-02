// This file is modified from uwebsockets example
/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include "test_def.h"

/* This is a simple WebSocket echo server example.
 * You may compile it with "WITH_OPENSSL=1 make" or with "make" */

int main(int argc, const char** argv) {

    if (argc != 3) {
        printf("Invalid parameters!\nUsage: ./echo_server port max_msg_len\n");
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


    /* ws->getUserData returns one of these */
    struct PerSocketData {
        /* Fill with user data */
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
            .maxPayloadLength = (uint32_t)max_msg_len,
            .idleTimeout = 16,
            .maxBackpressure = 100 * 1024 * 1024,
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend = false,
            .sendPingsAutomatically = true,
            /* Handlers */
            .upgrade = nullptr,
            .open = [](auto */*ws*/) {
                /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
                std::cout << "connection opened\n";
            },
            .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
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
    }).listen(test::listen_addr, port, [&](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << port << std::endl;
        }
    }).run();
}
