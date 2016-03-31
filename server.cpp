#include <unistd.h>
#include <iostream>
#include <string>
#include <memory>
#include "cache.h"
#include <exception>
#include "user_con.hpp"

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
        //this is normal, do nothing
    }
    /*catch(exception & unexpected_except){
        cout << unexpected_except.what();
       return 1;
    }*/

    return 0;
}
string make_json(string key,string value){
    return "{ \"key\": \"" + key + "\" , \"value\": \"" + value + "\" } ";
}
class safe_cache{
public:
    cache_t impl=nullptr;
    safe_cache(cache_t newc){
        impl = newc;
    }
    safe_cache(safe_cache &) = delete;
    ~safe_cache(){
        if(impl != nullptr){
            destroy_cache(impl);
            impl = nullptr;
        }
    }
    cache_t get(){
        return impl;
    }
};

class server_connection:
    public user_connection{
public:
    server_connection(asio::io_service & service,ip::tcp::acceptor & acceptor,uint64_t portnum):
        user_connection(service)
    {
        acceptor.accept(socket);
    }

    void act_on_message(safe_cache & cache, string message){
        size_t end_of_line = min(min(message.find('\n'),message.find('\r')),message.size()+1);

        size_t first_space = message.find(' ');
        size_t first_slash = message.find('/',first_space+1);
        size_t second_slash = message.find('/',first_slash+1);

        auto begining = message.begin();
        string fword = string(begining,begining+first_space);
        string info1 = string(begining+first_slash+1,begining+min(second_slash,end_of_line));
        string info2 = string(begining+second_slash+1,begining+end_of_line);
        if(fword == "GET"){
            uint32_t val_size = 0;
            val_type v = cache_get(cache.get(),(char *)(info1.c_str()),&val_size);
            if(v != nullptr){
                string output = make_json(info1,string((char *)(v)));
                write_message((char *)(output.c_str()),output.size());
            }else{
                return_error("got item not in cache");
            }
        }
        else if(fword == "PUT"){
            cache_set(cache.get(),(key_type)(info1.c_str()),(void*)(info2.c_str()),info2.size());
        }
        else if(fword == "DELETE"){
            cache_delete(cache.get(),info1.c_str());
        }
        else if(fword == "HEAD"){
            //todo:implement!
        }
        else if(fword == "POST"){
            if(info1 == "shutdown"){
                throw ExitException();
            }
            else if(info1 == "memsize"){
                if(cache_space_used(cache.get()) == 0){
                    cache = safe_cache(create_cache(stoll(info2),NULL));
                }
                else{
                    return_error("cache already created!");//todo: replace with valid HTTP message
                }
            }
            else{
                throw runtime_error("bad POST message");
            }
        }
        else{
            throw runtime_error("bad message");
        }
    }
};
void run_server(int portnum,int maxmem){
    asio::io_service my_io_service;
    safe_cache cache(create_cache(maxmem,nullptr));
    tcp::acceptor acceptor(my_io_service, tcp::endpoint(tcp::v4(), portnum));
    while(true){
        server_connection con(my_io_service,acceptor,portnum);
        con.act_on_message(cache,con.get_message());
    }
}
