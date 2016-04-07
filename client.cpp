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
ip::tcp::resolver tcp_resolver(my_io_service);
ip::udp::resolver udp_resolver(my_io_service);

string tcp_port = "9201";
string udp_port = "9202";
string serv_name = "localhost";

class client_udp_connection:
    public udp_connection{
public:
    client_udp_connection(asio::io_service & service,udp::endpoint & reciver):
        udp_connection(service)
    {
        socket.connect(reciver);
        set_block_timeout();
    }
};
class client_tcp_connection:
    public tcp_connection{
public:
    client_tcp_connection(asio::io_service & service,ip::tcp::resolver::iterator & resit):
        tcp_connection(service)
    {
        asio::connect(socket, resit);
    }
};

struct cache_obj{
    ip::tcp::resolver::iterator resit;
    udp::endpoint reciver;
    cache_obj(){
        resit = tcp_resolver.resolve({serv_name, tcp_port});
        reciver = *udp_resolver.resolve({serv_name, udp_port});
    }
    string send_message_tcp(bool getmes,string head,string word1,string word2=string()){
        client_tcp_connection con(my_io_service,resit);
        string finstr = head + " /" + word1 + (word2.size() == 0 ? "" : "/" + word2) + "\n";
        con.write_message((void*)(finstr.data()),finstr.size());
        return getmes ? con.get_message() : string();
    }
    string send_message_udp(bool getmes,string head,string word1,string word2=string()){
        client_udp_connection con(my_io_service,reciver);
        string finstr = head + " /" + word1 + (word2.size() == 0 ? "" : "/" + word2) + "\n";
        con.write_message(finstr);
        return getmes ? con.get_message() : string();
    }
};
void unpack_json(string json_str, string & key, string & value){
    json j = json::parse(json_str);
    key = j["key"];
    value = j["value"];
}

cache_t create_cache(uint64_t maxmem,hash_func h_fn){
    cache_t outc = new cache_obj();
    outc->send_message_tcp(false,"POST","memsize",to_string(maxmem));
    return outc;
}
void cache_set(cache_t cache, key_type key, val_type val, uint32_t val_size){
    char * val_str = (char *)(val);//val is assumed to be a string
    cache->send_message_tcp(false,"PUT",(char*)(key),string(val_str,val_str + val_size));
}
val_type cache_get(cache_t cache, key_type key, uint32_t *val_size){
    string keystr((char*)key);
    string retval = cache->send_message_udp(true,"GET",keystr);

    if(retval == errstr){
        *val_size = 0;
        cout << "ptr is null\n\n" << endl;
        return nullptr;
    }
    else{
        string valstr;
        unpack_json(retval,keystr,valstr);

        *val_size = valstr.size();
        //huge memory leak, but the only way I know how to do this correctly
        string * s = new string(valstr);
        string && str = std::move(valstr);
        s->swap(str);
        return s->data();
    }
}
void cache_delete(cache_t cache, key_type key){
    cache->send_message_tcp(false,"DELETE",string((char*)key));
}
uint64_t cache_space_used(cache_t cache){
    return 0;//not implemented
}
void destroy_cache(cache_t cache){
    cache->send_message_tcp(false,"POST","shutdown");
    delete cache;
}
