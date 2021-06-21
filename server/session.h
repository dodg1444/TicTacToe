#include <iostream>
#include <string>
#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "tools.h"


using namespace std;
using namespace boost::asio;
namespace asio = boost::asio;



class room;

class session : public enable_shared_from_this<session>{

public:
    using pointer = std::shared_ptr<session>;

    session(io_context& io_context);

    static pointer create(io_context& io_context);
    ip::tcp::socket& socket();

    string read();
    void send(string msg);
    void send_opponent(const shared_ptr<Query> q_ptr);
    void send_opponent(const string& msg);
    void send_roommates(string msg);
    void send_everyone(string msg); // except yourself
    void send_everyone(const shared_ptr<Query> q_ptr); // except yourself
    void start();
    void background_reading();
    void background_processing();
    void process_query(shared_ptr<Query>);
    void set_rooms_ptr(shared_ptr<thread_safe_unordered_map<string, shared_ptr<room>>>);
    void set_clients_ptr(shared_ptr<thread_safe_unordered_map<string, session::pointer>> clients);
    void set_id(const string&);
    void clean_up();
    string get_id();
    string get_username();
    bool is_ready();


private:
    ip::tcp::socket socket_;
    shared_ptr<thread_safe_unordered_map<string ,session::pointer>> clients_; // [id:session]
    shared_ptr<thread_safe_unordered_map<string ,shared_ptr<room>>> rooms_;
    shared_ptr<room> room_ptr_;
    threadsafe_q<Query> queries_;

    vector<future<void>> futures_;

    string username_ = "Default";
    string id_;
    string room_id_;

    atomic<bool> online_;
    atomic<bool> ready_;


};


class room {
public:
    room(session::pointer ptr);
    void add_player(session::pointer ptr);
    void remove_player(session::pointer ptr);
    bool is_full();
    bool is_empty();
    bool everyone_is_ready();
    void set_id(const string&);
    string form_query();

//private:

    shared_mutex mut_;
    string id_;
    session::pointer player_one_;
    session::pointer player_two_;

};























