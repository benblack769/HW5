#include <iostream>
#include <string>
#include "cache.h"
#include "user_con.hpp"
#include "json/json.hpp"

using namespace std;

using namespace asio;
using namespace asio::ip;

using json = nlohmann::json;

using asio::ip::tcp;

asio::io_service my_io_service;
ip::tcp::resolver resolver(my_io_service);

string my_port = "9200";


class client_connection:
    public user_connection{
public:
    client_connection(asio::io_service & service,ip::tcp::resolver::iterator & resit):
        user_connection(service)
    {
        asio::connect(socket, resit);
    }
};

struct cache_obj{
    ip::tcp::resolver::iterator resit;
    cache_obj(){
        resit = resolver.resolve({"localhost", my_port});
        //tcp::socket socket(my_io_service);
        //asio::connect(socket, resit);
       // cout << "connected" << endl;
    }
    string send_message(string head,string word1,string word2=string()){
        client_connection con(my_io_service,resit);
        cout << "connection established" << endl;
        string finstr = head + " /" + word1 + (word2.size() == 0 ? "" : "/" + word2) + "\n";
        con.write_message(finstr.data(),finstr.size());
        return con.get_message();
    }
};
void unpack_json(string json_str, string & key, string & value){
    json j = json::parse(json_str);
    key = j["key"];
    value = j["value"];
}

cache_t create_cache(uint64_t maxmem,hash_func h_fn){
    return new cache_obj();
}
void cache_set(cache_t cache, key_type key, val_type val, uint32_t val_size){
    char * val_str = (char *)(val);//val is assumed to be a string
    cache->send_message("PUT",key,string(val_str,val_str + val_size));
}
val_type cache_get(cache_t cache, key_type key, uint32_t *val_size){
    string retval = cache->send_message("GET",string(key));

    string keystr,valstr;
    unpack_json(retval,keystr,valstr);

    *val_size = valstr.size();
    return valstr.data();//todo:parse json
}
void cache_delete(cache_t cache, key_type key){
    string ignored_val = cache->send_message("DELETE",string(key));
}
uint64_t cache_space_used(cache_t cache){
    return 0;//not implemented
}
void destroy_cache(cache_t cache){
    cache->send_message("POST","shutdown");
    delete cache;
}
