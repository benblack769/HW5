#include <unistd.h>
#include <iostream>
#include <string>
#include <memory>
#include "cache.h"
#include <exception>
#include "user_con.hpp"
#include "serv_obj.hpp"

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
        run_server(portnum,8000,maxmem);
    }
    catch(ExitException & except){
        //this is normal, do nothing
    }
    catch(exception & unexpected_except){
        cout << unexpected_except.what();
       return 1;
    }

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

template<typename con_ty>
void get(con_ty & con,string key){
    uint32_t val_size = 0;
    val_type v = cache_get(cache.get(),(char *)(key.c_str()),&val_size);
    if(v != nullptr){
        string output = make_json(key,string((char *)(v)));
        con.write_message((char *)(output.c_str()),output.size());
    }else{
        con.return_error("got item not in cache");
    }
}
template<typename con_ty>
void put(con_ty & con,string key,string value){
    cache_set(cache.get(),(key_type)(key.c_str()),(void*)(value.c_str()),value.size());
}
template<typename con_ty>
void delete_(con_ty & con,string key){
    cache_delete(cache.get(),key.c_str());
}
template<typename con_ty>
void head(con_ty & con){
    //todo:implement!
}
template<typename con_ty>
void post(con_ty & con,string post_type,string extrainfo){
    if(post_type == "shutdown"){
        throw ExitException();
    }
    else if(post_type == "memsize"){
        if(cache_space_used(cache.get()) == 0){
            cache = safe_cache(create_cache(stoll(extrainfo),NULL));
        }
        else{
            con.return_error("cache already created!");//todo: replace with valid HTTP message
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
    string info2 = string(begining+second_slash+1,begining+end_of_line);
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
//the connection and server classes are mostly taken from the library documentation
class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(asio::io_service& io_service){
    return pointer(new tcp_connection(io_service));
  }

  tcp::socket& socket(){
    return socket_;
  }

  void start(){
    asio::async_read(socket,asio::buffer(buf,bufsize),boost::bind(&tcp_connection::handle_write, shared_from_this()))
  }
  void finalize(string & outmessage){
      asio::async_write(socket_, asio::buffer(outmessage),
          boost::bind(&tcp_connection::handle_write, shared_from_this()));
  }

private:
  tcp_connection(asio::io_service& io_service)
    : socket_(io_service)
  {
  }
  void handle_read(const asio::error_code& error){

  }

  void handle_write()
  {
  }

  tcp::socket socket_;
  char buf[bufsize];
};

class tcp_server
{
public:
  tcp_server(asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const asio::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
};

class udp_server
{
public:
  udp_server(asio::io_service& io_service)
    : socket_(io_service, udp::endpoint(udp::v4(), 13))
  {
    start_receive();
  }

private:
  void start_receive()
  {
    socket_.async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&udp_server::handle_receive, this,
          asio::placeholders::error));
  }

  void handle_receive(const asio::error_code& error)
  {
    if (!error){
      boost::shared_ptr<std::string> message(
          new std::string(make_daytime_string()));

      socket_.async_send_to(asio::buffer(*message), remote_endpoint_,
          boost::bind(&udp_server::handle_send, this, message));

      start_receive();
    }
  }

  void handle_send(boost::shared_ptr<std::string> /*message*/)
  {
  }
  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  boost::array<char, bufsize> recv_buffer_;
};

void run_server(int tcp_port,int udp_port,int maxmem){
    asio::io_service my_io_service;
    tcp::acceptor acceptor(my_io_service, tcp::endpoint(tcp::v4(), portnum));
    cache_connection con(my_io_service,acceptor,udp_port,tcp_port);
    while(true){
        act_on_message(con,con.get_message());
    }
}
/*
//
// blocking_udp_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <iostream>

using boost::asio::deadline_timer;
using boost::asio::ip::udp;

//----------------------------------------------------------------------

//
// This class manages socket timeouts by applying the concept of a deadline.
// Each asynchronous operation is given a deadline by which it must complete.
// Deadlines are enforced by an "actor" that persists for the lifetime of the
// client object:
//
//  +----------------+
//  |                |
//  | check_deadline |<---+
//  |                |    |
//  +----------------+    | async_wait()
//              |         |
//              +---------+
//
// If the actor determines that the deadline has expired, any outstanding
// socket operations are cancelled. The socket operations themselves are
// implemented as transient actors:
//
//   +---------------+
//   |               |
//   |    receive    |
//   |               |
//   +---------------+
//           |
//  async_-  |    +----------------+
// receive() |    |                |
//           +--->| handle_receive |
//                |                |
//                +----------------+
//
// The client object runs the io_service to block thread execution until the
// actor completes.
//
class client
{
public:
  client(const udp::endpoint& listen_endpoint)
    : socket_(io_service_, listen_endpoint),
      deadline_(io_service_)
  {
    // No deadline is required until the first socket operation is started. We
    // set the deadline to positive infinity so that the actor takes no action
    // until a specific deadline is set.
    deadline_.expires_at(boost::posix_time::pos_infin);

    // Start the persistent actor that checks for deadline expiry.
    check_deadline();
  }

  std::size_t receive(const boost::asio::mutable_buffer& buffer,
      boost::posix_time::time_duration timeout, boost::system::error_code& ec)
  {
    // Set a deadline for the asynchronous operation.
    deadline_.expires_from_now(timeout);

    // Set up the variables that receive the result of the asynchronous
    // operation. The error code is set to would_block to signal that the
    // operation is incomplete. Asio guarantees that its asynchronous
    // operations will never fail with would_block, so any other value in
    // ec indicates completion.
    ec = boost::asio::error::would_block;
    std::size_t length = 0;

    // Start the asynchronous operation itself. The handle_receive function
    // used as a callback will update the ec and length variables.
    socket_.async_receive(boost::asio::buffer(buffer),
        boost::bind(&client::handle_receive, _1, _2, &ec, &length));

    // Block until the asynchronous operation has completed.
    do io_service_.run_one(); while (ec == boost::asio::error::would_block);

    return length;
  }

private:
  void check_deadline()
  {
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expires_at() <= deadline_timer::traits_type::now())
    {
      // The deadline has passed. The outstanding asynchronous operation needs
      // to be cancelled so that the blocked receive() function will return.
      //
      // Please note that cancel() has portability issues on some versions of
      // Microsoft Windows, and it may be necessary to use close() instead.
      // Consult the documentation for cancel() for further information.
      socket_.cancel();

      // There is no longer an active deadline. The expiry is set to positive
      // infinity so that the actor takes no action until a new deadline is set.
      deadline_.expires_at(boost::posix_time::pos_infin);
    }

    // Put the actor back to sleep.
    deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

  static void handle_receive(
      const boost::system::error_code& ec, std::size_t length,
      boost::system::error_code* out_ec, std::size_t* out_length)
  {
    *out_ec = ec;
    *out_length = length;
  }

private:
  boost::asio::io_service io_service_;
  udp::socket socket_;
  deadline_timer deadline_;
};

//----------------------------------------------------------------------

int arg2(int argc, char* argv[])
{
  try
  {
    using namespace std; // For atoi.

    if (argc != 3)
    {
      std::cerr << "Usage: blocking_udp_timeout <listen_addr> <listen_port>\n";
      return 1;
    }

    udp::endpoint listen_endpoint(
        boost::asio::ip::address::from_string(argv[1]),
        std::atoi(argv[2]));

    client c(listen_endpoint);

    for (;;)
    {
      char data[1024];
      boost::system::error_code ec;
      std::size_t n = c.receive(boost::asio::buffer(data),
          boost::posix_time::seconds(10), ec);

      if (ec)
      {
        std::cout << "Receive error: " << ec.message() << "\n";
      }
      else
      {
        std::cout << "Received: ";
        std::cout.write(data, n);
        std::cout << "\n";
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}


template <typename MutableBufferSequence>
void read_with_timeout(tcp::socket& sock,
    const MutableBufferSequence& buffers)
{
  optional<error_code> timer_result;
  deadline_timer timer(sock.io_service());
  timer.expires_from_now(seconds());
  timer.async_wait(boost::bind(set_result, &timer_result, _1));
  optional<error_code> read_result;
  async_read(sock, buffers,
      boost::bind(set_result, &read_result, _1));

  sock.io_service().reset();
  while (sock.io_service().run_one())
  {
    if (read_result)
      timer.cancel();
    else if (timer_result)
      sock.cancel();
  }
  if (*read_result)
    throw system_error(*read_result);
}
*/
