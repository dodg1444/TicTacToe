#pragma once

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <future>
#include <memory>

#include "session.h"
#include "tools.h"

using namespace std;
using namespace boost::asio;
namespace asio = boost::asio;





class tcp_server{
public:
    explicit tcp_server(io_context& io_context);


private:
    void start_accept();
    void handle_accept(session::pointer new_session, const boost::system::error_code& error);

    io_context& io_context_;
    ip::tcp::acceptor acceptor_;

    vector<future<void>> fut_vect;
    shared_ptr<thread_safe_unordered_map<string ,session::pointer>> clients_; // [id:session]
    shared_ptr<thread_safe_unordered_map<string ,shared_ptr<room>>> rooms_;

};
























