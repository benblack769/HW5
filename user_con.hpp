#pragma once
#include <asio.hpp>
#include <vector>
#include <array>
//#include <bind/bind.hpp>
//#include <ctime>

using namespace asio;
using namespace asio::ip;

constexpr size_t bufsize = 1 << 15;

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
        write_message(myerr.c_str(),myerr.size());
    }
};

//class udp_connection;

//void handle_receive(const asio::error_code& ec, std::size_t length,
//                    udp_connection * con);
using bufarr = std::array<char,bufsize>;
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
bool make_str(std::vector<bufarr> & inbuf,std::string & out_buffstr,char delim){
    out_buffstr.clear();
    uint32_t count = 0;
    for(bufarr & buf : inbuf){
        uint32_t this_count = *reinterpret_cast<uint32_t*>(buf.data());
        if(this_count != count){
            return false;
        }
        for(size_t pos = sizeof(uint32_t); pos < buf.size();pos++){
            if(buf[pos] == delim){
                out_buffstr.insert(out_buffstr.end(),buf.begin()+sizeof(uint32_t),buf.begin()+delim);
                return true;
            }
        }
        out_buffstr.insert(out_buffstr.end(),buf.begin(),buf.end());
        count++;
    }
    return false;//didn't find delimiter
}
size_t find(bufarr & buf,char c){
    for(size_t pos = sizeof(uint32_t); pos < bufsize; pos++){
        if(buf[pos] == c){
            return pos;
        }
    }
    return std::string::npos;
}

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
        socket(service){}


    std::string get_message(){
        std::vector<bufarr> bufvec;
        while(true){
            bufvec.emplace_back();
            bufarr & buf = bufvec.back();
            size_t length = socket.receive(asio::buffer(buf));

            for(size_t pos = sizeof(uint32_t);pos < length; pos++){
                if(buf[pos] == '\n'){
                    break;
                }
            }
        }
        std::string message;
        if(make_str(bufvec,message,'\n'))
            return message;
        else
            return "";
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
