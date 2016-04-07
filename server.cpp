#include <unistd.h>
#include <iostream>
#include <string>
#include <memory>
#include "cache.h"
#include <exception>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <ostream>
#include "user_con.hpp"

using namespace std;

class ExitException: public exception
{
  virtual const char* what() const throw()
  {
    return "Exited cleanly";
  }
};
void run_server(int tcp_port, int udp_port, int maxmem);
int main(int argc, char ** argv){
    //take mainly off the get_opt wikipedia page
    int c;
    int tcp_portnum = 9201;
    int udp_portnum = 9202;
    int maxmem = 1 << 16;
    while ( (c = getopt(argc, argv, "m:p:u:")) != -1) {
        switch (c) {
        case 'm':
            maxmem = atoi(optarg);
            break;
        case 'p':
            tcp_portnum = atoi(optarg);
            break;
        case 'u':
            udp_portnum = atoi(optarg);
            break;
        default:
            cout << "bad command line argument";
            return 1;
        }
    }
    try{
        run_server(tcp_portnum,udp_portnum,maxmem);
    }
    catch(ExitException &){
        //this is normal, do nothing
    }
    catch(exception & unexpected_except){
        cout << unexpected_except.what();
       return 1;
    }

    return 0;
}
string make_json(string key,string value){
    key.pop_back();
    return "{ \"key\": \"" + key + "\" , \"value\": \"" + value + "\" } \n";
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

template<typename con_ty>
void get(con_ty & con,string key){
    uint32_t val_size = 0;
    val_type v = cache_get(con.cache(),(key_type)(key.c_str()),&val_size);
    if(v != nullptr){
        string output = make_json(key,string((char *)(v)));
        con.write_message(output);
    }else{
        con.return_error();
    }
}
template<typename con_ty>
void put(con_ty & con,string key,string value){
    cache_set(con.cache(),(key_type)(key.c_str()),(void*)(value.c_str()),value.size());
}
template<typename con_ty>
void delete_(con_ty & con,string key){
    cout << "val deleted" << endl;
    cache_delete(con.cache(),(key_type)(key.c_str()));
}
template<typename con_ty>
void head(con_ty &){
    //todo:implement!
}
template<typename con_ty>
void post(con_ty & con,string post_type,string extrainfo){
    if(post_type == "shutdown"){
        throw ExitException();
    }
    else if(post_type == "memsize"){
        if(cache_space_used(con.cache()) == 0){
            destroy_cache(con.cache());
            con.cache() = create_cache(stoll(extrainfo),NULL);
        }
        else{
            con.return_error();
        }
    }
    else{
        throw runtime_error("bad POST message");
    }
}
template<typename con_ty>
void act_on_message(con_ty & con, string message){
    size_t end_of_line = min(min(message.find('\n'),message.find('\r')),message.size()+1);

    size_t first_space = message.find(' ');
    size_t first_slash = message.find('/',first_space+1);
    size_t second_slash = message.find('/',first_slash+1);

    auto begining = message.begin();
    string fword = string(begining,begining+first_space);
    string info1 = string(begining+first_slash+1,begining+min(second_slash,end_of_line));
    string info2 = second_slash == string::npos ? string() : string(begining+second_slash+1,begining+end_of_line);

    if(fword == "GET"){
        get(con,info1);
    }
    else if(fword == "PUT"){
        put(con,info1,info2);
    }
    else if(fword == "DELETE"){
        delete_(con,info1);
    }
    else if(fword == "HEAD"){
        head(con);
    }
    else if(fword == "POST"){
        post(con,info1,info2);
    }
    else{
        throw runtime_error("bad message");
    }
}
string to_string(asio::streambuf & b){
    asio::streambuf::const_buffers_type bufs = b.data();
    return std::string(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + b.size());
}
//the connection and server classes are mostly taken from the library documentation
class tcp_con
  : public boost::enable_shared_from_this<tcp_con>
{
public:
  typedef boost::shared_ptr<tcp_con> pointer;

  static pointer create(asio::io_service& io_service,safe_cache * cache){
    return pointer(new tcp_con(io_service,cache));
  }

  tcp::socket& socket(){
    return socket_;
  }

  void start(){
      asio::async_read_until(socket_,b,'\n',
                         boost::bind(&tcp_con::handle_read,shared_from_this(),asio::placeholders::error(),asio::placeholders::bytes_transferred()));

  }
  void write_message(string s){
      asio::async_write(socket_, asio::buffer(s),
                        boost::bind(&tcp_con::handle_write, shared_from_this()));
  }
  void return_error(){
      write_message("ERROR");
  }
  cache_t & cache(){
      return port_cache->impl;
  }
  tcp_con(asio::io_service& io_service,safe_cache * in_cache)
    : socket_(io_service),
      port_cache(in_cache)
  {
  }

  void handle_read(const asio::error_code& error,
                   size_t bytes_written){
      act_on_message(*this, to_string(b));
  }

  void handle_write()
  {
  }

  tcp::socket socket_;
  safe_cache * port_cache;//non-owning
  asio::streambuf b;
};

class tcp_server
{
public:
  tcp_server(asio::io_service& io_service,int portnum,safe_cache & in_cache)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), portnum)),
      port_cache(&in_cache)
  {
  }

  void start_accept()
  {
    tcp_con::pointer new_connection =
      tcp_con::create(acceptor_.get_io_service(),port_cache);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          asio::placeholders::error));
  }
private:

  void handle_accept(tcp_con::pointer new_connection,
      const asio::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
  safe_cache * port_cache;//non-owning
};
class udp_server
{
public:
  udp_server(asio::io_service& io_service,int udp_port,safe_cache & incache)
    : socket_(io_service,udp::endpoint(udp::v4(), udp_port)),
      port_cache(&incache)
  {
  }
  void write_message(string s){
      vector<bufarr> bufvec;
      make_buf_vec(bufvec,s);

      for(bufarr & arr : bufvec){
        socket_.async_send_to(asio::buffer(arr),endpoint,
                   boost::bind(&udp_server::handle_send,this,asio::placeholders::error(),asio::placeholders::bytes_transferred()));
      }
  }
  void return_error(){
      write_message("ERROR\n");
  }
  cache_t & cache(){
      return port_cache->impl;
  }

  void start_receive()
  {
      socket_.async_receive_from(asio::buffer(buf),endpoint,
                          boost::bind(&udp_server::handle_receive, this,
                          asio::placeholders::error,asio::placeholders::bytes_transferred()));
  }
  void handle_receive(const asio::error_code& error,size_t bytes_written)
  {
    if (!error){
        //can be made more optimal with a special case for get_packet_num(buf) == 0
        if(get_packet_num(buf) != packet_count){
            packet_count = 0;
            recv_data.clear();
        }
        else if(add_to_str_finished(buf,recv_data,'\n')){
            packet_count = 0;
            act_on_message(*this,recv_data);
            recv_data.clear();
        }
        else{
            packet_count += 1;
        }
    }
    start_receive();
  }
  void handle_send(const asio::error_code& ,size_t )
  {
  }
  udp::socket socket_;
  udp::endpoint endpoint;
  safe_cache * port_cache;//non-owning
  string recv_data;
  bufarr buf;
  uint32_t packet_count = 0;
};

void run_server(int tcp_port,int udp_port,int maxmem){
    asio::io_service my_io_service;
    safe_cache serv_cache(create_cache(maxmem,nullptr));

    tcp_server tcon(my_io_service,tcp_port,serv_cache);
    udp_server ucon(my_io_service,udp_port,serv_cache);

    tcon.start_accept();
    ucon.start_receive();

    my_io_service.run();
}
