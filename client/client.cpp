#include "client.h"



client::client(io_context& io_context, const string& hostname, const string& port) :
                                socket_(io_context), resolver_(io_context){
    tcp::resolver::query query(/*move(*/hostname/*)*/, /*move(*/port/*)*/);
    tcp::endpoint remote_endpoint = *resolver_.resolve(query);
    socket_.connect(remote_endpoint);
    online_.store(true);

    queries_to_send_ = make_shared<threadsafe_q<Query>>();

    service_msg(msg_type::INFO,  "Connected to " ,remote_endpoint.address(), ":", remote_endpoint.port());

}


void client::send(string msg){
    boost::system::error_code write_error;

    size_t buffer_size = 1024;

    if (msg.size() > buffer_size)
        buffer_size = msg.size();

    const char* msg_ = msg.c_str();

    boost::asio::write(socket_, buffer(msg_, buffer_size), write_error); // doesn't work with buffer size lesser than 1024

    if (!write_error){
//        service_msg(msg_type::DEBUG, "Succesfully sent to server: ", msg);
    }

    else {
        ui_->set_title(L"Server is offline");
//        service_msg(msg_type::DEBUG,  "Failed writing to socket: ", write_error.message());
        online_.store(false);
    }
}


string client::read(){

    char read_buffer[1024];
    boost::system::error_code read_error;

    boost::asio::read(socket_, buffer(read_buffer, 1024), /*boost::asio::transfer_all(), */read_error);

    if (!read_error){
        return string(read_buffer);
    }

    else {
        ui_->set_title(L"Server is offline");
        online_.store(false);
        return "Error";
    }
}


// main loop
void client::init(const string& nickname){

    futures.push_back(reallyAsync(&client::async_read, this));
    futures.push_back(reallyAsync(&client::async_write, this));

    ui_ = make_unique<drawing_device>();
    ui_->set_queue_ptr(queries_to_send_);
    ui_->set_nickname(nickname);

    for (const auto& fut : futures){
        fut.wait();
    }
}



void client::parse_input(string msg){

    if (msg == "Error")
        return;

//    Query query(msg);
//    auto query_ptr = make_shared<Query>(msg);

//    ui_->queries_to_process_.push(query);
    ui_->process_query(make_shared<Query>(msg));


//    service_msg(msg_type::INFO, "Parsed: ", msg);
}


void client::async_read(){
    while(online_.load())
        parse_input(read());
}


void client::async_write(){
    while(online_.load()){
//        string msg = stringify(queries_to_send_->wait_and_pop());
//        ui_->set_title(wstring(msg.begin(), msg.end()));
        send(stringify(queries_to_send_->wait_and_pop()));
    }
}


string client::stringify(shared_ptr<Query> query){
    return query->type_ + "," + query->sender_ + "," + query->data_;
}
























