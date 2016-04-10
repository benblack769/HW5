#pragma once
#include <asio.hpp>
#include <vector>
#include <array>
#include <unistd.h>
//#include <bind/bind.hpp>
//#include <ctime>

using namespace asio;
using namespace asio::ip;

constexpr size_t bufsize = 1 << 15;
const std::string errstr = "ERROR";

//class udp_connection;

//void handle_receive(const asio::error_code& ec, std::size_t length,
//                    udp_connection * con);
using bufarr = std::array<char,bufsize>;

size_t find_in_buf(bufarr & buf,size_t startpos,char c){
    for(size_t pos = startpos; pos < bufsize; pos++){
        if(buf[pos] == c){
            return pos;
        }
    }
    return std::string::npos;
}
