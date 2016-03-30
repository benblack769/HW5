#include <asio.hpp>
#include <unistd.h>
#include <iostream>
#include <string>
#include "cache.h"

using namespace std;

using namespace asio;
using namespace asio::ip;

void run_server(int portnum,int maxmem);
int main(int argc, char ** argv){
    //take mainly off the get_opt wikipedia page
    int c;
    int portnum = 9200;
    int maxmem = 1 << 16;
    while ( (c = getopt(argc, argv, "m:p:")) != -1) {
        switch (c) {
        case 'm':
            maxmem = atoi(optarg);
            break;
        case 'p':
            portnum = atoi(optarg);
            break;
        }
    }
    //try{
        run_server(portnum,maxmem);
    //}
    //catch{

    //}
    return 0;
}
string get_message(tcp::socket & socket){
    const uint64_t max_length = 1 << 16;
    string message;
    while(true){
        char buffer[max_length];

        asio::error_code error;
        size_t length = sock.read_some(asio::buffer(data,max_length), error);
        if (error == asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw asio::system_error(error); // Some other error.

        message.insert(message.end(),buffer,buffer+length);
    }
    return message;
}
void write_message(tcp::socket & socket,void * buf,size_t len){
    asio::error_code error;
    asio::write(socket, asio::buffer(buf,len), error);
}
void act_on_message(tcp::socket & socket,string message){
    size_t first_endline = message.find_first_of('\n');
    string first_line = string(message.begin(),message.begin()+first_endline);
    if(first_line == "GET"){
        write_message(socket, (string(in_message,in_message+length) + out_message).c_str());
    }
    else{
        exit(1);
    }
}

void run_server(int portnum,int maxmem){
    asio::io_service my_io_service;

    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::resolver::query query("localhost", "http");

    ip::tcp::acceptor acceptor(my_io_service, tcp::endpoint(tcp::v4(), portnum));

    //ip::tcp::socket socket(my_io_service);
    //acceptor.accept(socket);
    tcp::socket socket(my_io_service);

    acceptor.accept(socket);
    while(true){
        std::string out_message = "hi there. How am I doing?";
        char in_message[10000] = {0};
        asio::error_code ignored_error;
        uint64_t length = asio::read(socket, asio::buffer(in_message,10),ignored_error);
        asio::write(socket, asio::buffer(string(in_message,in_message+length) + out_message), ignored_error);
    }
}
