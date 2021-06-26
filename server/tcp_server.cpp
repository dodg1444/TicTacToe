#include "tcp_server.h"


tcp_server::tcp_server(io_context& io_context, const int& port) : io_context_(io_context),
    acceptor_(io_context, ip::tcp::endpoint(ip::tcp::v4(), port))
{

    clients_ = make_shared<thread_safe_unordered_map<string, session::pointer>>();
    rooms_ = make_shared<thread_safe_unordered_map<string, shared_ptr<room>>>();
    debug_msg("Started listening on 127.0.0.0:", port);
    start_accept();

}


void tcp_server::start_accept(){

    debug_msg("Started async accept");

    auto new_session = session::create(io_context_);
    string new_session_id = to_string(GetRandInt());

    new_session->set_clients_ptr(clients_);
    new_session->set_rooms_ptr(rooms_);
    new_session->set_id(new_session_id);


    clients_->add_pair(new_session_id, new_session);

    acceptor_.async_accept(new_session->socket(), boost::bind(
                               &tcp_server::handle_accept, this, new_session,
                               boost::asio::placeholders::error));

}



void tcp_server::handle_accept(session::pointer new_session, const boost::system::error_code& error){

    if (!error){
        fut_vect.push_back(reallyAsync(&session::start, new_session));
    }

    start_accept();
}























