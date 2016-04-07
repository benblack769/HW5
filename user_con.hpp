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

class tcp_connection{
public:
    tcp::socket socket;
    tcp_connection(asio::io_service & service):
        socket(service){}

    std::string get_message(){
        const uint64_t max_length = 1 << 16;
        std::string message;
        while(true){
            char buffer[max_length];
            usleep(10);
            asio::error_code error;
            size_t length = socket.read_some(asio::buffer(buffer,max_length), error);
            if (error == asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw asio::system_error(error); // Some other error.

            message.insert(message.end(),buffer,buffer+length);

            if(message.find('\n') != std::string::npos){
                break;
            }
        }
        return message;
    }
    void write_message(void * buf,size_t len){
        asio::error_code error;
        asio::write(socket, asio::buffer(buf,len), error);
        if (error){
            throw asio::system_error(error);
        }
    }
    void return_error(std::string myerr){
        write_message((char*)(myerr.c_str()),myerr.size());
    }
};

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
class sock_opt_h{
public:
    timeval time;
    sock_opt_h(){
        time.tv_sec = 0;
        time.tv_usec = 50 * 1000;
    }
    int level(const udp &) const{
        return SOL_SOCKET;
    }
    int name(const udp &)const{
        return SO_RCVTIMEO;
    }
    void * data(const udp &)const{
        return (void*)(&time);
    }
    //const timeval &
    size_t size(const udp &)const{
        return sizeof(time);
    }
};
class udp_connection{
public:
    udp::socket socket;
    /*io_service * io_service_;

    static constexpr size_t max_length = 1 << 16;
    char buffer[max_length];
    size_t read_length;
    asio::error_code read_err;

    size_t timed_recive(char * buffer,size_t max_length,asio::error_code error){
        //size_t length = socket.receive(asio::buffer(buffer,max_length),0, error);
        socket.async_receive(asio::buffer(buffer,max_length),
                             boost::bind(handle_receive,_1,_2,this));
        while()
    }*/

    udp_connection(asio::io_service & service):
        socket(service){
    }

    void set_block_timeout(){
        sock_opt_h def;
        socket.set_option(def);
    }

    std::string get_message(){
        uint32_t packet_count = 0;
        std::string recv_data;
        while(true){
            bufarr buf;
            size_t length = socket.receive(asio::buffer(buf));

            if(get_packet_num(buf) != packet_count){
                return errstr;
            }
            else if(add_to_str_finished(buf,recv_data,'\n')){
                return recv_data;
            }
            else{
                packet_count += 1;
            }
        }
    }


    void write_message(std::string s){
        std::vector<bufarr> bufvec;
        make_buf_vec(bufvec,s);
        for(bufarr & buf : bufvec){
            socket.send(asio::buffer(buf));
        }
    }
    void return_error(std::string myerr){
        write_message(myerr);
    }
};
/*
void handle_receive(const asio::error_code& ec, std::size_t length,
                    udp_connection * con){
    con->read_length = length;
    con->read_err = ec;
}*/
