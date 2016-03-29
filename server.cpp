#include <asio.hpp>
#include <algorithm>

int main(){
    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::resolver::query query("www.boost.org", "http");

    ip::tcp::socket socket(my_io_service);
    asio::connect(socket, resolver.resolve(query));

    ip::tcp::socket socket(my_io_service);
    socket.connect(endpoint);


    ip::tcp::acceptor acceptor(my_io_service, my_endpoint);
    ...
    ip::tcp::socket socket(my_io_service);
    acceptor.accept(socket);
}
