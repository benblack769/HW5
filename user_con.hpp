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

void make_buf_vec(std::vector<bufarr> & outbuf,std::string & buffstr){
    outbuf.clear();
    uint32_t count = 0;
    uint64_t str_buf_idx = 0;
    constexpr size_t av_size = bufsize-sizeof(uint32_t);
    do{
        outbuf.emplace_back();
        auto & buf = outbuf.back();
        *reinterpret_cast<uint32_t*>(buf.data()) = count;
        std::copy(buffstr.begin()+str_buf_idx,buffstr.begin()+str_buf_idx+av_size,buf.begin()+sizeof(uint32_t));
        str_buf_idx += av_size;
    }while(str_buf_idx < buffstr.size());
}
uint32_t get_packet_num(bufarr & buf){
    return *reinterpret_cast<uint32_t*>(buf.data());
}

bool add_to_str_finished(bufarr & buf,std::string & out_str,char delim){
    const size_t pk_size = sizeof(uint32_t);
    size_t loc = find_in_buf(buf,pk_size,delim);
    if(loc == std::string::npos){
        out_str.insert(out_str.end(),buf.begin()+pk_size,buf.end());
        return false;
    }
    else{
        out_str.insert(out_str.end(),buf.begin()+pk_size,buf.begin()+loc);
        return true;
    }
}
