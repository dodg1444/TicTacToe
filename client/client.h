#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "drawing_device.h"
#include "tools.h"

using boost::asio::ip::tcp;
using namespace boost::asio;
using namespace std;

class client{
public:
    client(io_context& io_context, const string& hostname, const string& port);
    client(const client&) = delete;
    client& operator=(const client&) = delete;
    void send(string msg);
    string read();
    void init(const string& nickname);
    void parse_input(string);
    void async_read();
    void async_write();
    string stringify(shared_ptr<Query>);

private:
    tcp::socket socket_;
    tcp::resolver resolver_;
    unique_ptr<drawing_device> ui_;
    vector<future<void>> futures;
    shared_ptr<threadsafe_q<Query>> queries_to_send_;
    atomic<bool> online_;
};
