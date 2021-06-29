#include "session.h"


using pointer = shared_ptr<session>;


session::session(io_context& io_context) : socket_(io_context){
    online_ = true;
    get_time();
    cout << "New client connected\n";
}


pointer session::create(io_context& io_context){
    return pointer(new session(io_context));
}


ip::tcp::socket& session::socket(){
    return socket_;
}


string session::read(){

    boost::system::error_code read_error;
    char read_buffer[1024];


    boost::asio::read(socket_, buffer(read_buffer, 1024), read_error);


    if (!read_error){
        debug_msg("Received from client: ", read_buffer);

        return string(read_buffer);
    }

    else{
        debug_msg("Failed reading from socket\n");
        online_.store(false);
        get_time();
        cout << "Client disconnected\n";
        return "Error on read";
    }
}


void session::send(const string& msg){
    boost::system::error_code write_error;
    size_t buffer_size = 1024;

    if (msg.size() > buffer_size)
        buffer_size = msg.size();

    const char* msg_chars = msg.c_str();

    boost::asio::write(socket_, buffer(msg_chars, buffer_size), write_error); // doesn't work with buffer size lesser than 1024


    if (!write_error){
        debug_msg("Sent ", msg, " to client");
    }

    else{
        debug_msg("Failed writing to socket: " + write_error.message());
    }
}




void session::start(){

    futures_.push_back(reallyAsync(&session::background_reading, this));
    futures_.push_back(reallyAsync(&session::background_processing, this));

    for (const auto& fut : futures_)
        fut.wait();
}


void session::background_reading(){
    while (online_.load()){
        string data = read();
        Query q(data);
        queries_.push(q);
    }
    clean_up();
}


void session::background_processing(){
    while (online_.load()){
        process_query(queries_.wait_and_pop());
    }

}

//         query patterns
// host_game {0, none, none}                    not used on client side
// join_game {1, room_id_32123, none}           not used on client side
// change_name {2, new_name, none}              not used on client side
// finish_turn {3, none, none}
// chat_msg {4, Frank, sdfsfsdf}
// exit {5, none, none}                         not used on client side
// leave_game {6, none, none}
// ready {7, none, none}
// unready {8, none, none}
// opponent_joined {9, Frank, none}
// start_fight {10, none, none}
// get_hosted_rooms_list {11, none, none}
// add_hosted_room {12, room_id, Dodg&Joe}
// remove_hosted_room {13, room_id, none}
// player_joined {14, Frank, not_ready}
// chose_cell {15, 0-8, none}
// your_turn {16, none, none}
// you_lost {17,none,none}
// restart_fight {18, none, none}


void session::process_query(shared_ptr<Query> q_ptr){
    int q_type = stoi(q_ptr->type_);
    switch (q_type) {
        case(0) : {
            room_id_ = to_string(GetRandInt());
            room_ptr_ = make_shared<room>(clients_->get_value(id_));
            rooms_->add_pair(room_id_, room_ptr_);
            room_ptr_->set_id(room_id_);

            send_everyone("12,"+room_id_+","+username_);



            // add new room to lobby TS_unordered_map
            // add player to the room
            break;
        }
        case(1) : {
            string room_id = q_ptr->sender_;
            if (rooms_->contains(room_id)){
                room_id_ = room_id;
                room_ptr_ = rooms_->get_value(room_id_);
                room_ptr_->add_player(clients_->get_value(id_));

                // player_joined {14, Frank, not_ready}
                room_ptr_->player_one_->send("14,"+room_ptr_->player_two_->username_+","+to_string(room_ptr_->player_two_->is_ready()));
                room_ptr_->player_two_->send("14,"+room_ptr_->player_one_->username_+","+to_string(room_ptr_->player_one_->is_ready()));

                // inform opponent about join
            }

            break;
        }
        case(2) : {
            username_ = q_ptr->sender_;
            debug_msg("Changed username to: ", username_);
            break;
        }
        case(3) : {
            // just pass the query to an opponent
            break;
        }
        case(4) : {
            send_everyone(q_ptr);
            debug_msg("New message from you: ", q_ptr->data_);

            // just pass the query to an opponent
            break;
        }
        case(5) : {
            clean_up();
            break;
        }
        case(6) : {
            // inform opponent about leave

//            room_ptr_ = rooms_->get_value(room_id_);
            send_opponent(q_ptr);
            room_ptr_->remove_player(clients_->get_value(id_));
            if (room_ptr_->is_empty()){
                rooms_->erase_if_present(room_id_);

                // remove_hosted_room {13, room_id, none}
                send_everyone("13,"+room_id_+",none");
            }

            room_id_ = "";
            room_ptr_ = nullptr;
            debug_msg(username_, " left game room");
            debug_msg("Rooms available: ", rooms_->size());
            break;
        }
        case(7) : {
            ready_.store(true);
            debug_msg(username_, " is ready to rip and tear until it's done");
            room_ptr_ = rooms_->get_value(room_id_);

            if (room_ptr_->is_full()){
                send_opponent(q_ptr);
                if (room_ptr_->everyone_is_ready()){
                    send_roommates("10, none, none");

                    // send first turn
                    if (GetRandInt() > 0)
                        send_opponent("16,none,none");

                    else
                        send("16,none,none");
                }
            }
            break;
        }
        case(8) : {
            ready_.store(false);

            if (room_ptr_->is_full())
                send_opponent(q_ptr);

            debug_msg(username_, " is not ready");
            break;
        }

        case(11) : {

            for (const auto& room : rooms_->get_values())
                send(room->form_query());

            break;
        }
        case(15) : {
            send_opponent(q_ptr);
            send_opponent("16,none,none");
            break;
        }
        case(17) : { // you lost
            send_opponent(q_ptr);
            break;
        }
        case(18) : { // restart
            send_opponent(q_ptr);

            if (GetRandInt() > 0)
                send_opponent("16,none,none");

            else
                send("16,none,none");


            break;
        }
        case(19) : { // tie
            send_opponent(q_ptr);
            break;
        }
    }
}

void session::set_rooms_ptr(shared_ptr<thread_safe_unordered_map<string ,shared_ptr<room>>> ptr){
    rooms_ = ptr;
}

void session::set_clients_ptr(shared_ptr<thread_safe_unordered_map<string, session::pointer>> clients){
    clients_ = clients;
};


void session::set_id(const string& id){
    id_ = id;
}


void session::clean_up(){
    clients_->erase_if_present(id_);
    rooms_->erase_if_present(room_id_);
};


void session::send_everyone(const string& msg){
    auto clients = clients_->get_values();

    for (const auto& client : clients){
        if (client->get_id() != id_)
            client->send(msg);
    }
}

void session::send_everyone(const shared_ptr<Query> q_ptr){
    auto clients = clients_->get_values();

    for (const auto& client : clients){
        if (client->get_id() != id_)
            client->send(q_ptr->type_+","+q_ptr->sender_+"," + q_ptr->data_);
    }
}


string session::get_id(){
    return id_;
};

bool session::is_ready(){
    return ready_.load();
}

void session::send_roommates(const string& msg){
    auto room_ptr = rooms_->get_value(room_id_);
    room_ptr->player_one_->send(msg);
    room_ptr->player_two_->send(msg);
};


string session::get_username(){
    return username_;
};

void session::send_opponent(const shared_ptr<Query> q_ptr){

     if (room_ptr_->is_full()){
         if (room_ptr_->player_one_->id_ == id_)
             room_ptr_->player_two_->send(q_ptr->type_+","+q_ptr->sender_+"," + q_ptr->data_);

         else
             room_ptr_->player_one_->send(q_ptr->type_+","+q_ptr->sender_+"," + q_ptr->data_);
     }

};

void session::send_opponent(const string& msg){
    if (room_ptr_->is_full()){
        if (room_ptr_->player_one_->id_ == id_)
            room_ptr_->player_two_->send(msg);

        else
            room_ptr_->player_one_->send(msg);

    }
};





room::room(session::pointer ptr) : player_one_(ptr){};

void room::add_player(session::pointer ptr){

    unique_lock<shared_mutex> lk(mut_);
    if (player_one_)
        player_two_ = ptr;

    else
        player_one_ = ptr;
}


void room::remove_player(session::pointer ptr){
    unique_lock<shared_mutex> lk(mut_);
    if (player_one_ == ptr)
        player_one_ = nullptr;

    else
        player_two_ = nullptr;
};


bool room::is_full(){
    shared_lock<shared_mutex> lk(mut_);
    return player_one_ && player_two_;
};


bool room::is_empty(){
    shared_lock<shared_mutex> lk(mut_);
    return !(player_one_ || player_two_);
}


bool room::everyone_is_ready(){
    shared_lock<shared_mutex> lk(mut_);
    return player_one_->is_ready() && player_two_->is_ready();
};


void room::set_id(const string& room_id){
    id_ = room_id;
};


string room::form_query(){
    string query = "12,";
    query += id_ + ",";
    if (is_full()){
        query += player_one_->get_username();
        query += "&";
        query += player_two_->get_username();
    }

    else {
        if (player_one_)
            query += player_one_->get_username();

        if (player_two_)
            query += player_two_->get_username();
    }
    return query;
};

















