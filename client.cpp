#include <asio.hpp>
#include <algorithm>

using asio::ip::tcp;

int main(){
    asio::io_service io_service;

    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::resolver::query query("www.boost.org", "http");

    ip::tcp::socket socket(my_io_service);
    asio::connect(socket, resolver.resolve(query));

}
