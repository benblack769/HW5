#include <asio.hpp>
#include <iostream>
#include <string>

using namespace std;

using namespace asio;
using namespace asio::ip;

using asio::ip::tcp;

/*
struct cache_obj{
    asio::io_service service;
    ip::tcp::socket socket;
};
cache_t create_cache(uint64_t maxmem,hash_func h_fn){

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
*/
int main(){
    asio::io_service my_io_service;

    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::socket socket(my_io_service);
    asio::connect(socket, resolver.resolve({"localhost", "8230"}));
    while(true){
        std::string req = "this is a request";
        cout << "client writing" << endl;
        asio::write(socket, asio::buffer(req));
        char out[1000];
        cout << "client reading" << endl;
        asio::read(socket, asio::buffer(out,10));

    }
}
