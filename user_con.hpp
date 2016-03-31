#pragma once
#include <asio.hpp>

class user_connection{
public:
    ip::tcp::acceptor acceptor;
    tcp::socket socket;

    user_connection(asio::io_service & service,uint64_t portnum):
        acceptor(service, tcp::endpoint(tcp::v4(), portnum)),
        socket(service)
    {
        acceptor.accept(socket);
    }
    string get_message(){
        const uint64_t max_length = 1 << 16;
        string message;
        while(true){
            char buffer[max_length];

            asio::error_code error;
            size_t length = socket.read_some(asio::buffer(buffer,max_length), error);
            if (error == asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw asio::system_error(error); // Some other error.

            message.insert(message.end(),buffer,buffer+length);

            if(message.find('\n') != string::npos){
                break;
            }
        }
        return message;
    }
    void write_message(tcp::socket & socket,void * buf,size_t len){
        asio::error_code error;
        asio::write(socket, asio::buffer(buf,len), error);
    }
    void return_error(string myerr){
        write_message(socket,myerr.c_str(),myerr.size());
    }
};
