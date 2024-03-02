#pragma once

namespace test {

//    constexpr const char* const listen_addr = "10.5.96.3";
//    constexpr const char* const listen_addr = "10.5.96.7";
    inline constexpr const char* cert_file_path = "./certs/server.crt";
    inline constexpr const char* key_file_path = "./certs/server.key";
    inline constexpr const char* ca_file_path = "./certs/ca.pem";

//    inline constexpr size_t MAX_DATA_LEN = 1 << 16;
//    inline constexpr size_t MAX_DATA_LEN = 64;

} // namespace test