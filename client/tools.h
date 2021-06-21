#pragma once

#include <iostream>
#include <future>
#include <queue>
#include <sstream>
#include <string>
#include <memory>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <algorithm>

using namespace std;




enum class msg_type {DEBUG, INFO};
// prints every thing you feed it
template <typename Arg, typename... Args>
inline void service_msg(const msg_type& type, Arg&& arg, Args&&... args)
{
    switch (type) {
        case(msg_type::DEBUG) : {
            cout << "DEBUG: ";
            break;
        }
        case(msg_type::INFO) : {
            cout << "INFO: ";
            break;
        }
    }
    cout << std::forward<Arg>(arg);
    using expander = int[];
    (void)expander{0, (void(cout << std::forward<Args>(args)), 0)...};
    cout << "\n";
}





// wrapper for async
// Thanks Scott
template <typename F, typename... Ts>
inline
auto // C++14 way
//future<typename result_of<F(Ts...)>::type> // C++11 way
reallyAsync(F&& f, Ts&&... params){
    return async(launch::async, forward<F>(f), forward<Ts>(params)...);
}


// char('1') -> int(1)
inline int ctoi(const char& ch){
   return ch - '0' ;
}


template <typename Map, typename Key>
void
inline
delete_if_present(Map& m, const Key& key){
    if (m.count(key))
        m.erase(key);
}









template <typename T>
class threadsafe_q {

private:
    mutable mutex mut;
    queue<shared_ptr<T>> data_q;
    condition_variable data_cond;

public:
    threadsafe_q(){};
    void push(T new_value){
        shared_ptr<T> data(make_shared<T>(move(new_value)));
        lock_guard<mutex> lk(mut);
//        cout << "Pushed " << *data << " into the q\n";
        data_q.push(data);
        data_cond.notify_one();
    }


    void wait_and_pop(T& value){
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_q.empty();});
        value = move(*data_q.front());
        data_q.pop();
    }


    shared_ptr<T> wait_and_pop(){
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_q.empty();});
        shared_ptr<T> res = data_q.front();
        data_q.pop();
//        cout << "Popped " << *res << " from the q\n";
        return res;
    }


    bool try_pop(T& value){
        lock_guard<mutex> lk(mut);
        if(data_q.empty())
            return false;
        value = move(*data_q.front());
        data_q.pop();
        return true;
    }


    shared_ptr<T> try_pop(){
        lock_guard<mutex> lk(mut);
        if (data_q.empty())
            return shared_ptr<T>();
        shared_ptr<T> res = data_q.front();
        data_q.pop();
        return res;
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return data_q.empty();
    }
};





class Query {
public:
    Query(string msg){
        stringstream ss(msg);
        getline(ss, type_, ',');
        getline(ss, sender_, ',');
        getline(ss, data_);
    }

//private:
    string type_;
    string sender_;
    string data_;
};


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
// tie {19,none,none}

inline ostream& operator<<(ostream& os, const Query& q){
    os << q.type_ << "," << q.data_;
    return os;
}


























