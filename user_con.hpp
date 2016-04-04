#pragma once
#include <asio.hpp>
//#include <bind/bind.hpp>
//#include <ctime>

using namespace asio;
using namespace asio::ip;

class tcp_con{
public:
    tcp::socket socket;
    tcp_con(asio::io_service & service):
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
        const size_t max_length = 1 << 16;
        char buffer[max_length];
        std::string message;
        while(true){

            asio::error_code error;
            size_t length = socket.receive(asio::buffer(buffer,max_length),0, error);
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
        socket.send(asio::buffer(buf,len), 0,error);
    }
    void return_error(std::string myerr){
        write_message(myerr.c_str(),myerr.size());
    }
};
/*
void handle_receive(const asio::error_code& ec, std::size_t length,
                    udp_connection * con){
    con->read_length = length;
    con->read_err = ec;
}*/
