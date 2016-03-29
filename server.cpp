#include <asio.hpp>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

using namespace asio;
using namespace asio::ip;

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
    run_server(portnum,maxmem);
    return 0;
}
void run_server(int portnum,int maxmem){
    asio::io_service my_io_service;

    ip::tcp::resolver resolver(my_io_service);
    ip::tcp::resolver::query query("www.boost.org", "http");

    ip::tcp::acceptor acceptor(my_io_service, tcp::endpoint(tcp::v4(), portnum));

    ip::tcp::socket socket(my_io_service);
    acceptor.accept(socket);
}
