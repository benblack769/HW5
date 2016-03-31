#include <asio.hpp>
#include <unistd.h>
#include <iostream>
#include <string>
#include "cache.h"

using namespace std;

using namespace asio;
using namespace asio::ip;
#include <exception>
using namespace std;

class ExitException: public exception
{
  virtual const char* what() const throw()
  {
    return "Exited cleanly";
  }
};
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
    try{
        run_server(portnum,maxmem);
    }
    catch(ExitException & except){
        //this is normal
    }
    catch(exception & unexpected_except){
        cout << unexpected_except.what();
        return 1;
    }

    return 0;
}
string make_json(string key,string value){
    return "{ key: " + key + ", value: " + value + " } ";
}
class port_listener{
public:
    tcp::socket socket;
    ip::tcp::acceptor acceptor;
    port_listener(asio::io_service & service,uint64_t portnum):
        acceptor(service, tcp::endpoint(tcp::v4(), portnum)),
        socket(service),
        cache(port_cache)
    {
        acceptor.accept(socket);
    }
    ~port_listener(){
        if(cache != nullptr){
            destroy_cache(cache);
        }
    }
    void act_on_message(string message){
        size_t first_space = message.find(' ');
        size_t first_slash = message.find('/',first_space+1);
        size_t second_slash = message.find('/',message.find(char(0),first_slash)+1);
        second_slash = (second_slash == string::npos) ? message.size()+1 : second_slash;
        size_t final_term = message.find('/',message.find(char(0),first_slash)+1);

        auto begining = message.begin();
        string fword = string(begining,begining+first_space);
        string info1 = string(begining+first_slash,begining+second_slash);
        string info2 = string(begining+second_slash,message.end());
        if(fword == "GET"){
            uint32_t val_size = 0;
            val_type v = cache_get(cache,(char *)(info1.c_str()),&val_size);
            string output = make_json(info1,string((char *)(v)));
            write_message(socket,(char *)(output.c_str()),output.size());
        }
        else if(fword == "PUT"){
            cache_set(cache,(key_type)(info1.c_str()),(void*)(info2.c_str()),info2.size());
        }
        else if(fword == "DELETE"){
            cache_delete(cache,info1.c_str());
        }
        else if(fword == "HEAD"){
            //todo:implement!
        }
        else if(fword == "POST"){
            if(info1 == "shutdown"){
                throw ExitException();
            }
            else if(info1 == "memsize"){
                if(cache == nullptr){
                    cache = create_cache(stoll(info1),NULL);
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

void run_server(int portnum,int maxmem){
    asio::io_service my_io_service;
    cache_t cache = nullptr;
    while(true){
        port_listener port(cache,my_io_service,portnum);
        port.act_on_message(port.get_message());
    }
}
