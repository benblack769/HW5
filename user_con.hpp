#pragma once
#include <asio.hpp>
#include <vector>
#include <array>
#include <unistd.h>
//#include <bind/bind.hpp>
//#include <ctime>

using namespace asio;
using namespace asio::ip;

constexpr size_t bufsize = (1 << 16) - 32;
const std::string errstr = "ERROR";

//class udp_connection;

//void handle_receive(const asio::error_code& ec, std::size_t length,
//                    udp_connection * con);
using bufarr = std::array<char,bufsize>;

size_t find_in_buf(bufarr & buf,char c){
    for(size_t pos = 0; pos < bufsize; pos++){
        if(buf[pos] == c){
            return pos;
        }
    }
    return std::string::npos;
}
bool is_white_space(char c){
    //first one is non-standard, but useful in my application
    return c == char(0) || c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

void strip(std::string & str){
    size_t strs = str.size();
    for(int i = strs-1; i >= 0;i--){
        if(!is_white_space(str[i])){
            str.resize(i+1);
            break;
        }
    }
}
