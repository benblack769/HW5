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
bool is_beginning_of(string big,string little){
    for(int i = 0; i < little.size(); i++){
        if(big[i] != little[i]){
            return false;
        }
    }
    return true;
}
string make_json(string key,string value){
    return "{ key: " + key + ", value: " + value + " } "
}
class port_listener{
public:
    tcp::socket socket;
    ip::tcp::acceptor acceptor;
    cache_t cache = nullptr;
    port_listener(asio::io_service & service,uint64_t portnum){
        ip::tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), portnum));

        tcp::socket socket(service);
        acceptor.accept(socket);
    }
    ~port_listener(){
        if(cache != nullptr){
            cache_delete(cache);
        }
    }
    void act_on_message(asio::io_service & my_io_service,string message){
        size_t first_space = message.find(' ');
        size_t first_slash = message.find('/',first_space+1);
        size_t second_slash = message.find('/',message.find(0,first_slash)+1);
        second_slash = (second_slash == string::npos) ? message.size()+1 : second_slash;
        size_t final_term = message.find('/',message.find(0,first_slash)+1);

        auto begining = message.begin();
        string fword = string(begining,begining+first_space);
        string info1 = string(begining+first_slash,begining+second_slash);
        sstring info2 = string(begining+second_slash,message.end());
        if(fword == "GET"){
            uint32_t val_size = 0;
            val_type v = cache_get(cache,info1.c_str(),&val_size);
            string output = make_json(info1,string((char *)(v)));
            write_message(socket,output.c_str(),output.size());
        }
        else if(fword == "PUT"){
            cache_set(cache,info1.c_str(),info2.c_str(),info2.size());
        }
        else if(fword == "DELETE"){
            cache_delete(cache,info1.c_str());
        }
        else if(fword == "HEAD"){
            //todo:implement!
        }
        else if(fword == "POST"){
            if(info1 == "shutdown"){

            }
            else if(info1 == "memsize"){
                if(cache == nullptr){
                    cache = create_cache(strtoull(info1),NULL);
                }
                else{
                    return_error("cache already created!");
                }
            }
        }
        else{
            cout << "bad message" << endl;
            exit(1);
        }
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
    void return_error(tcp::socket & socket,string myerr){
        write_message(socket,myerr.c_str(),myerr.size());
    }
}

void run_server(int portnum,int maxmem){
    asio::io_service my_io_service;

    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::resolver::query query("localhost", "http");

    ip::tcp::acceptor acceptor(my_io_service, tcp::endpoint(tcp::v4(), portnum));

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
