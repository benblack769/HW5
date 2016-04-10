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

class tcp_connection{
public:
    tcp::socket socket;
    tcp_connection(asio::io_service & service,ip::tcp::resolver::iterator & resit):
        socket(service){

        asio::connect(socket, resit);
    }

    std::string get_message(){
        const uint64_t max_length = 1 << 16;
        std::string message;
        while(true){
            char buffer[max_length];
            usleep(10);
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
        write_message((char*)(myerr.c_str()),myerr.size());
    }
};

class sock_opt_h{
public:
    timeval time;
    sock_opt_h(){
        time.tv_sec = 0;
        time.tv_usec = 50 * 1000;
    }
    int level(const udp &) const{
        return SOL_SOCKET;
    }
    int name(const udp &)const{
        return SO_RCVTIMEO;
    }
    void * data(const udp &)const{
        return (void*)(&time);
    }
    //const timeval &
    size_t size(const udp &)const{
        return sizeof(time);
    }
};
class udp_connection{
public:
    udp::socket socket;

    udp_connection(asio::io_service & service,udp::endpoint & reciver):
        socket(service){

        socket.connect(reciver);
        set_block_timeout();
    }

    void set_block_timeout(){
        sock_opt_h def;
        socket.set_option(def);
    }

    std::string get_message(){
        uint32_t packet_count = 0;
        std::string recv_data;
        while(true){
            bufarr buf;
            size_t length = socket.receive(asio::buffer(buf));

            if(get_packet_num(buf) != packet_count){
                return errstr;
            }
            else if(add_to_str_finished(buf,recv_data,'\n')){
                return recv_data;
            }
            else{
                packet_count += 1;
            }
        }
    }


    void write_message(std::string s){
        std::vector<bufarr> bufvec;
        make_buf_vec(bufvec,s);
        for(bufarr & buf : bufvec){
            socket.send(asio::buffer(buf));
        }
    }
    void return_error(std::string myerr){
        write_message(myerr);
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
        tcp_connection con(my_io_service,resit);
        string finstr = head + " /" + word1 + (word2.size() == 0 ? "" : "/" + word2) + "\n";
        con.write_message((void*)(finstr.data()),finstr.size());
        return getmes ? con.get_message() : string();
    }
    string send_message_udp(bool getmes,string head,string word1,string word2=string()){
        udp_connection con(my_io_service,reciver);
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
    if(val_size > bufsize){
        return;
    }
    char * val_str = (char *)(val);//val is assumed to be a string
    cache->send_message_tcp(false,"PUT",(char*)(key),string(val_str,val_str + val_size));
}
val_type cache_get(cache_t cache, key_type key, uint32_t *val_size){
    string keystr((char*)key);
    string retval = cache->send_message_udp(true,"GET",keystr);

    if(retval == errstr){
        *val_size = 0;
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
