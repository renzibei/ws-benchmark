#pragma once
#include <cstdint>
#include <cstddef>

#define SERVER_PORT     58600
//#define MAX_DATA_LEN    (1 << 16)
#define MAX_DATA_LEN 64

//#define TEST_TIMES      1'00'000

#define ENABLE_NO_DELAY 1
#define ENABLE_BUSY_POLL 1
#define BUSY_POLL_US 800
#define SET_NON_BLOCK 1
#define SET_LINGER_ZERO_TIMEOUT 0

#define MAX_WRITE_EVENT_WRITE_SIZE 65536
#define FSTACK_ONE_TIME_FULL_THRES 16384
#define MAX_FSTACK_ONE_TIME_WRITE_SIZE 16384

#define SEND_LATENCY_DATA 1

inline constexpr int SSL = 1;
inline constexpr const char* cert_file_path = "/root/codes/ws-benchmark/certs/server.crt";
inline constexpr const char* key_file_path = "/root/codes/ws-benchmark/certs/server.key";
inline constexpr const char* ca_file_path = "/root/codes/ws-benchmark/certs/ca.pem";

namespace test {


// aws-vi01
//inline constexpr const char * SERVER_IP  = "172.31.86.246";
//inline constexpr const char* SERVER_IP = "172.31.99.99";
//inline constexpr const char * SERVER_IP  = "127.0.0.1";

// vu-la01
//inline constexpr const char* const SERVER_IP = "10.5.96.3";
//inline constexpr const char* const SERVER_IP = "108.61.219.248";

// vu-la06

    inline constexpr const char *SERVER_IP = "10.5.96.7";

    inline constexpr size_t MSG_LIMIT_PER_CLIENT = 50;
//    inline constexpr size_t MSG_LIMIT_PER_CLIENT = 1;

    inline constexpr size_t CON_CLIENT_NUM = 500;

    inline constexpr size_t REBORN_LIMIT_FOR_CLIENT = 10;

    inline constexpr size_t TOTAL_MSG_CNT = MSG_LIMIT_PER_CLIENT * CON_CLIENT_NUM * REBORN_LIMIT_FOR_CLIENT;

#if SEND_LATENCY_DATA
    inline constexpr size_t LATENCY_DATA_SIZE = sizeof(uint64_t) * 2;
#else
    inline constexpr size_t LATENCY_DATA_SIZE = 0;
#endif

    static_assert(LATENCY_DATA_SIZE <= MAX_DATA_LEN);


#define MAX_EVENT_NUM 512

} //namespace test