// This file is modified from uwebsockets example
/* We simply call the root header file "App.h", giving you uWS::App and uWS::SSLApp */
#include "App.h"
#include "test_def.h"

/* This is a simple WebSocket echo server example.
 * You may compile it with "WITH_OPENSSL=1 make" or with "make" */

int main(int argc, const char** argv) {

    if (argc < 4) {
        printf("Invalid parameters!\nUsage: ./echo_server ip_address port max_msg_len\n");
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
            .maxPayloadLength = (uint32_t)MAX_DATA_LEN,
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
    }).listen(SERVER_IP, SERVER_PORT, [&](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << SERVER_PORT << std::endl;
        }
    }).run();
}
