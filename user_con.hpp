#pragma once
#include <asio.hpp>

using namespace asio;
using namespace asio::ip;

class user_connection{
public:
    tcp::socket socket;
    user_connection(asio::io_service & service):
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
