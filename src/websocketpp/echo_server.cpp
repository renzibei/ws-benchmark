#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>



uint64_t last_interval_send_bytes = 0;
uint64_t last_interval_recv_bytes = 0;
uint64_t last_interval_recv_msg_cnt = 0;
uint64_t last_interval_send_msg_cnt = 0;
size_t last_msg_size = 0;
int64_t interval_start_ns = 0;
int64_t active_connections = 0;

size_t MAX_DATA_LEN = 0;
FILE* output_fp = nullptr;

void count_stats(){
    if (((last_interval_recv_msg_cnt & 0xfffffUL)
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





typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

void on_open(websocketpp::connection_hdl /*hdl*/) {
    ++active_connections;
}

void on_close(websocketpp::connection_hdl /*hdl*/) {
    --active_connections;
}

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
//    std::cout << "on_message called with hdl: " << hdl.lock().get()
//              << " and message: " << msg->get_payload()
//              << std::endl;

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
//    if (msg->get_payload() == "stop-listening") {
//        s->stop_listening();
//        return;
//    }
    size_t msg_size = msg->get_payload().size();
    if (interval_start_ns == 0) {
        interval_start_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    last_interval_recv_bytes += msg_size;
    ++last_interval_recv_msg_cnt;


    try {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
        // please note that send does not mean all sent to the wire
        last_msg_size = msg_size;
        last_interval_send_bytes += msg_size;
        ++last_interval_send_msg_cnt;
        count_stats();
    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

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
    MAX_DATA_LEN = size_t(max_msg_len);

    const char* export_file_path = argv[4];
    output_fp = fopen(export_file_path, "w");
    if (output_fp == nullptr) {
        printf("Cannot create file at %s\n", export_file_path);
        return -1;
    }
    printf("Will output to %s\n", export_file_path);
    // msg_size, rx goodput, tx goodput, rx mm mps, tx mm mps
    fprintf(output_fp, "msg_size,rx_goodput,tx_goodput,rx_mm_mps,tx_mm_mps,connection_cnt\n");



    printf("Prepare to run\n");
    // Create a server endpoint
    server echo_server;

    try {
        // Set logging settings

        echo_server.clear_access_channels(websocketpp::log::alevel::all);
//        echo_server.set_access_channels(websocketpp::log::alevel::fail);

        // Initialize Asio
        echo_server.init_asio();
        echo_server.set_reuse_addr(true);

        // Register our message handler
        echo_server.set_message_handler(bind(&on_message,&echo_server,::_1,::_2));
//        echo_server.set_message_handler(on_message);

        echo_server.set_open_handler(on_open);
        echo_server.set_close_handler(on_close);

        // Listen on port 9002
        echo_server.listen(std::string(SERVER_IP), std::to_string(SERVER_PORT));

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        echo_server.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }

    return 0;
}