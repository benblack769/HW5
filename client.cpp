#include <iostream>
#include <string>
#include "cache.h"
#include "user_con.hpp"

using namespace std;

using namespace asio;
using namespace asio::ip;

using asio::ip::tcp;

asio::io_service my_io_service;
ip::tcp::resolver resolver(my_io_service);

string my_port = "9200";

struct cache_obj{
    ip::tcp::resolver::iterator resit;
    cache_obj(){
        resit = resolver.resolve({"localhost", "9200"});
    }
    void send_message(string head,string word1,string word2){

    }
};

cache_t create_cache(uint64_t maxmem,hash_func h_fn){
    return new cache_obj();
}
void cache_set(cache_t cache, key_type key, val_type val, uint32_t val_size){

}
val_type cache_get(cache_t cache, key_type key, uint32_t *val_size){

}
void cache_delete(cache_t cache, key_type key){

}
uint64_t cache_space_used(cache_t cache){

}
void destroy_cache(cache_t cache){
    delete cache;
}

int arg(){
    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::socket socket(my_io_service);
    ip::tcp::resolver::iterator resit = resolver.resolve({"localhost", "9200"});
    asio::connect(socket, resit);
    while(true){
        std::string req = "this is a request";
        cout << "client writing" << endl;
        asio::write(socket, asio::buffer(req));
        char out[1000];
        cout << "client reading" << endl;
        asio::read(socket, asio::buffer(out,10));

    }
}
