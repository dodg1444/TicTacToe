#include <iostream>
#include <functional>  // for function
#include <string>    // for wstring, allocator, basic_string
#include <vector>    // for vector
                        // for the king and the kingdom!

#include "drawing_device.h"



using namespace ftxui;


drawing_device::drawing_device() : screen_(ScreenInteractive::TerminalOutput()){
    init_ui();
    screen_loop_ = reallyAsync(&ScreenInteractive::Loop, &screen_, layout_);
}


void drawing_device::init_ui(){


////////////////////////////////// MENU ///////////////////////////////////////////

    stage_ = game_stage::menu;
    menu_ = Menu(&menu_entries_names_, &selected_entry_);
    MenuBase::From(menu_)->on_enter = bind(&drawing_device::event_handler, this);
    MenuBase::From(menu_)->normal_style = color(Color::Red) | bold;
    MenuBase::From(menu_)->focused_style = color(Color::Red) | bold;
    MenuBase::From(menu_)->selected_style = color(Color::Blue);
    draw_menu();
    //    MenuBase::From(layout_)->selected_focused_style = bold | color(Color::Blue);

////////////////////////////////// CHAT ///////////////////////////////////////////

    chat_input_ = Input(&chat_msg_, "");
    InputBase::From(chat_input_)->on_enter = [&](){
        send(Query("4,"+ to_string(nickname_) + "," + to_string(chat_msg_)));
        Chat_msg msg(nickname_, chat_msg_);
        chat_messages_.push_back(msg);
        chat_msg_.clear();
    };

    chat_printed_messages_ = Renderer([&]{

        // might write overload for queue of Elements for chat representation
        for (auto& msg : chat_messages_){

            if (elems_.size() > number_of_shown_messages_)
                elems_.erase(elems_.begin()); // it is higly inefficient to remove first element from a vector, but I haven't written an
                                              // overload for another container of Elements (for example queue)

            if (msg.from(nickname_))
                elems_.push_back(msg.form_text(Chat_msg::alignment::left));
//                elems_.emplace(elems_.begin(), msg.form_text(Chat_msg::alignment::left));

            else
                elems_.push_back(msg.form_text(Chat_msg::alignment::right));
//                elems_.emplace(elems_.begin(), msg.form_text(Chat_msg::alignment::right));
        }

        return vbox({move(elems_)});
    });


    chat_components_ = Container::Vertical({
                          chat_printed_messages_,
                          chat_input_
                                           });



////////////////////////////////// CHANGE_NAME ////////////////////////////////////

    change_name_widget_ = Input(&new_name_, "");
    InputBase::From(change_name_widget_)->on_enter = [&]{
        drawn_tab_number_ = 0;
        draw_settings();

        send(Query("2,"+to_string(new_name_) + ",none"));
        nickname_ = new_name_;
        new_name_.clear();
    };




    change_name_container_ = Container::Vertical({
                                                     change_name_widget_
                                                 });

    change_name_tab_ = Renderer(change_name_container_, [&]{
        return vbox({
                        window(text(L"New username:"), change_name_widget_->Render()) | color(Color::Yellow3)
                    });

    });


////////////////////////////////// EXIT ///////////////////////////////////////////

    yes_button_text_ = L"Yes";

    yes_button_ = Button(&yes_button_text_, [&]{
        if (let_out_)
            terminate(); // a dirty way to stop a program execution but it'll do for now


        yes_button_text_ = L"There is no coming back...";
        let_out_ = true;
    });


    no_button_ = Button("Nah just kidding", [&]{
        draw_menu();
        drawn_tab_number_ = 0;
    });

    exit_container_ = Container::Horizontal({
        yes_button_,
        no_button_,
    });

    exit_tab_ = Renderer(exit_container_, [&] {
        return vbox({
                    text(L"Already leaving?...") | size(HEIGHT, EQUAL, number_of_shown_messages_),
                   hbox({
                       yes_button_->Render(),
                       filler(),
                       no_button_->Render(),
                   }),
               });
      });


//////////////////////////////// HOST GAME ////////////////////////////////////////

    ready_toggle_ = Toggle(&toggle_entries_, &my_selection_);
    ToggleBase::From(ready_toggle_)->on_enter = [&](){
        if (!my_selection_){
            set_title(L"You are ready to rip and tear until it's done");
            send(Query("7,none,none"));
        }
        else {
            set_title(L"You're not ready to smash");
            send(Query("8,none,none"));

        }
    };

    leave_game_button_ = Button("Cowardly run away", [&]{
        send(Query("8,none,none"));
        send(Query("6,none,none"));
        draw_menu();
        drawn_tab_number_ = 0;
    });


    host_game_container_ = Container::Vertical({
                                                   ready_toggle_,
                                                   leave_game_button_
                                               });

    host_game_tab_ = Renderer(host_game_container_, [&]{
        return vbox({
                        hbox(text(L" * " + nickname_ + L"      "), align_right(ready_toggle_->Render())),
                        hbox(text(L" * " + opponent_nickname_ + L"      "), align_right(text(opponent_status_))),
                        filler() | size(HEIGHT, EQUAL, number_of_shown_messages_),
                        hbox(hcenter(leave_game_button_->Render()))
                    });
    });



//////////////////////////////// JOIN GAME ////////////////////////////////////////


    rooms_list_ = Menu(&hosted_rooms_, &selected_room_);

    // TO_DO:
    MenuBase::From(rooms_list_)->on_enter = [&](){
        draw_host_game();
        drawn_tab_number_ = 3;
        // join_game {1, room_id_32123, none}
        int temp = selected_room_;
        temp++;
        send(Query("1,"+rooms_map_[temp]->id_+",none"));
        // join room by number -> send id to the server;
        // draw in_room tab
        // get data about an opponent - readiness etc.

    };


    leave_lobby_button_ =  Button("Back to menu", [&]{
        draw_menu();
        drawn_tab_number_ = 0;
    });

    join_game_container_ = Container::Vertical({
                                                   rooms_list_,
                                                   leave_lobby_button_
                                               });

    join_game_tab_ = Renderer(join_game_container_, [&]{
        return vbox({
                       rooms_list_->Render(),
                       filler() | size(HEIGHT, EQUAL, number_of_shown_messages_ - hosted_rooms_.size()),
                       leave_lobby_button_->Render()
                    });
    });





////////////////////////////////// FIGHT //////////////////////////////////////////

    my_turn_.store(false);
    in_fight_.store(false);
    spin_.store(false);

    opponent_pressed_button_symbol_ = L"    O";
    pressed_button_symbol_ = L"    X"; // change it to 'O' if needed
    not_pressed_button_symbol_ = L"     ";

    for (int i = 0; i < 9; ++i){
        ints_.push_back(i);
        symbols_.push_back(not_pressed_button_symbol_);
        cell_colors_.push_back(cells_color_);
    }


    for (const auto& index : ints_){
        cells_.push_back(Button(&symbols_[index], [&]{
            if (my_turn_.load()){
                if (symbols_[index] == not_pressed_button_symbol_){
                    // send info about pressed button
                    send(Query("15," + to_string(index) + ",none"));
                    symbols_[index] = pressed_button_symbol_;
                    field_.take_cell(index);
                    my_turn_.store(false);
                    ++turns_done_;
                    int won_ = won();
                    if (won_){
                        set_victory_cells_color(won_);

                        left_column_text_ = L"Victory is ours!";

                        // you_lost {17,123,none}
                        // send info
                        send(Query("17,"+ to_string(won_) +",none"));
                        return;
                    }

                    if (turns_done_ == 5){
                        left_column_text_ = L"Tie!";
                        send(Query("19, none,none"));
                        return;
                    }


                    left_column_text_ = L"Opponent's turn";
                }
            }
        }));
    }



    exit_button_text_ = L"Leave";

    leave_fight_button_ = Button(exit_button_text_, [&]{
        send(Query("6,none,none"));
        draw_menu();
        drawn_tab_number_ = 0;
    });


    restart_fight_button_ = Button(L"Restart", [&]{
        // send restart
        send(Query("18, none, none"));

        // reset buttons and vars
        reset_game_field();
    });

    spinner_ = Renderer([&]{
        return hbox({
                        spinner(21,spinner_index_) | color(Color::Yellow3),
//                        spinner(2,spinner_index_+1),
//                        spinner(2,spinner_index_+2),
//                        spinner(2,spinner_index_+3)
                    });
});



    fight_container_ = Container::Vertical({
                                               cells_[0],cells_[1],cells_[2],
                                               cells_[3],cells_[4],cells_[5],
                                               cells_[6],cells_[7],cells_[8],
                                              leave_fight_button_,
                                              spinner_,
                                              restart_fight_button_
                                           });

    fight_tab_ = Renderer(fight_container_, [&]{
        return vbox({
                    // game field
                    vbox({
                            hbox({
                            cells_[0]->Render() | flex_grow | color(cell_colors_[0]),
//                                filler(),
                            cells_[1]->Render() | flex_grow | color(cell_colors_[1]),
//                                filler(),
                            cells_[2]->Render() | flex_grow | color(cell_colors_[2]),
                        }),
                        hbox({
                            cells_[3]->Render()| flex_grow | color(cell_colors_[3]),
//                                filler(),
                            cells_[4]->Render()| flex_grow | color(cell_colors_[4]),
//                                filler(),
                            cells_[5]->Render()| flex_grow | color(cell_colors_[5]),
                        }),
                        hbox({
                            cells_[6]->Render()| flex_grow | color(cell_colors_[6]),
//                                filler(),
                            cells_[7]->Render()| flex_grow | color(cell_colors_[7]),
//                                filler(),
                            cells_[8]->Render()| flex_grow | color(cell_colors_[8]),
                        })
                    }) | border ,
                    filler() | yflex_grow,
//                        yflex_grow(),
                    text(L""),
                    hbox({
                            leave_fight_button_->Render(),
                            filler(),
                            spinner_->Render(),
                            filler(),
                            restart_fight_button_->Render()
                        })
                });
    });









////////////////////////////////// MAIN ///////////////////////////////////////////


    screen_updater_ = reallyAsync([&](){
        while(true){

            if (spin_.load())
                spinner_index_++;

            screen_.PostEvent(Event::Custom);
            this_thread::sleep_for(update_every_);
        }
    });

    left_side_content_ = Container::Tab({
                                            menu_,
                                            change_name_tab_,
                                            exit_tab_,
                                            host_game_tab_,
                                            join_game_tab_,
                                            fight_tab_
                                        }, &drawn_tab_number_);




    container_ = Container::Vertical({left_side_content_, chat_components_});


    layout_ = Renderer(container_, [&]{
        return vbox({
                    vbox({
                        hbox({
                        hcenter(bold(text(upper_text_)))}),
                        separator(),
                        hbox({
                            vbox({hcenter(bold(text(left_column_text_))) | color(main_color_),
                                  separator(),
                                  left_side_content_->Render(),
                            }) | flex,
                            separator(),
                            vbox({hcenter(bold(text(L"Chat"))) | color(main_color_),
                                  separator(),
                                  chat_printed_messages_->Render() | size(HEIGHT, EQUAL, number_of_shown_messages_)
                                  ,
                                  window(text(L"Message: "), chat_input_->Render()) | color(Color::Yellow3)
                            }) | flex /*| size(WIDTH, EQUAL, 40) */,
                        })
                    })| border | color(Color::BlueViolet) /*| size(HEIGHT, LESS_THAN, 40)*/
                });
            }) ;
}


void drawing_device::draw_menu(){
    update_every_ = 0.250s;
    stage_ = game_stage::menu;
    menu_entries_names_ = {L"Host game", L"Join game", L"Settings", L"Exit"};
    upper_text_ = L"Welcome to Tic-Tac-Toe";
    left_column_text_ = L"Menu";
    spin_.store(false);
}


void drawing_device::draw_settings(){
    menu_entries_names_ = {L"Change name", L"Change theme", L"Github", L"Back"};
    upper_text_ = L"Here you can adjust stuff. That's right. No need to thank me";
    left_column_text_ = L"Settings";
};


void drawing_device::draw_change_name(){
    drawn_tab_number_ = 1;
    upper_text_ = L"Here you can change your name. That's right. No need to thank me";
    left_column_text_ = L"Change name";
}


void drawing_device::draw_exit(){
    set_title(L"Wanna leave?");
    upper_text_ = L"Exit";
    drawn_tab_number_ = 2;

}


void drawing_device::draw_host_game(){
    in_fight_.store(true);
    left_column_text_ = L"Room";
    set_title(L"Waiting for an opponent...");
    drawn_tab_number_ = 3;
}


void drawing_device::draw_join_game(){
    stage_ = game_stage::lobby;
    set_title(L"Join game");
    left_column_text_ = L"Hosted games";
    drawn_tab_number_ = 4;
};

void drawing_device::draw_fight(){

    update_every_ = 0.125s;
    in_fight_.store(true);
    spin_.store(true);
    set_title(L"Let the battle begin");
    left_column_text_ = L"Opponent's turn";
    drawn_tab_number_ = 5;
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
// you_lost {17,123,none} // paint losing cells
// restart_fight {18, none, none}
// tie {19,none,none}

void drawing_device::event_handler(){
//    cout << "Event handler invoked\n";
    switch (stage_) {
        case(game_stage::menu) : {
            switch (selected_entry_) {
                case(0) : { // host_game {0, none, none}
                    send(Query("0, none, none"));
                    draw_host_game();
                    break;
                }
                case(1) : {
                    rooms_map_.clear();
                    hosted_rooms_.clear();
                    send(Query("11, none, none"));
                    draw_join_game();
                    break;
                }
                case(2) : {
                    stage_ = game_stage::settings;
                    draw_settings();
                    break;
                }
                case(3) : {
                    draw_exit();
                    break;
                }
            }
            break;
        }
        case(game_stage::settings) : {
            switch (selected_entry_) {
                case(0) : {
                    draw_change_name();
                    break;
                }
                case(1) : {
                    set_title(L"In development");
                    this_thread::sleep_for(2s);
                    draw_settings();
                    break;
                }
                case(2) : { // git
#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>

                    ShellExecute(0, 0, L"https://github.com/dodg1444/TicTacToe", 0, 0 , SW_SHOW );

#elif __linux__
                    system("open https://github.com/dodg1444/TicTacToe > /dev/null");
#endif

                    break;
                }
                case(3) : {
                    stage_ = game_stage::menu;
                    draw_menu();
                    break;
                }
            }
            break;
        }
        default: {
//            cout << "Unknown stage\n";
            break;
        }
    }

    selected_entry_ = 0;
}


void drawing_device::set_queue_ptr(const shared_ptr<threadsafe_q<Query>> ptr){
    queries_to_send_ = ptr;
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
// you_lost {17,123,none} // paint losing cells
// restart_fight {18, none, none}
// tie {19,none,none}


void drawing_device::process_query(shared_ptr<Query> q_ptr){
    int q_type = stoi(q_ptr->type_);
    switch (q_type) {
        case(4) : {
        //        if(__receive_world_messages_flag__)
        //            process_this_query();
            Chat_msg msg(q_ptr->sender_, q_ptr->data_);
            chat_messages_.push_back(msg);
            break;
        }
        case(6) : { // leave_game {6, none, none}



            if (in_fight_.load()){
                reset_game_field();
                draw_host_game();
            }

            set_title(L"Waiting for an opponent...");
            opponent_nickname_ = L"Waiting...";
            opponent_status_ = L"Waiting...";

            break;
        }
        case(7) : { // ready {7, none, none}
            opponent_status_ = L"Ready";


            break;
        }
        case(8) : { // unready {8, none, none}
            opponent_status_ = L"Not ready";

            break;
        }
        case(9) : { // opponent_joined
            break;
        }
        case(10) : { // start_fight
            draw_fight();
            break;
        }
        case(12) : {
        // add_hosted_room {12, room_id, Dodg&Joe}

            // get number of the last element
            int counter = 1;
            if (rooms_map_.empty())
                rooms_map_.insert(make_pair(1, make_shared<Room>(q_ptr->sender_, to_wstring(q_ptr->data_))));

            else {
                auto it = rooms_map_.end();
                counter = it->first;
                ++counter;
                // insert new element in the end
                rooms_map_.insert(make_pair(counter, make_shared<Room>(q_ptr->sender_, to_wstring(q_ptr->data_))));
            }

            // update list of rooms accordingly
            hosted_rooms_.push_back(to_wstring(counter) + L". " + to_wstring(q_ptr->data_));
            break;
        }
        case(13) : {
            rooms_map_.clear();
            hosted_rooms_.clear();
            send(Query("11, none, none"));
//            remove_room_by_id(q_ptr->sender_);
                    // remove_hosted_room {13, room_id, none}
            break;
        }
        case(14) : { // player_joined {14, Frank, not_ready}
            opponent_nickname_ = to_wstring(q_ptr->sender_);

            if (q_ptr->data_ == "0")
                opponent_status_ = L"Not ready";

            else
                opponent_status_ = L"Ready";


            break;
        }
        case(15) : {
            // chose_cell {15, 0-8, none}

//            set_title(to_wstring(q_ptr->data_));

            // disable the button somehow...
            symbols_[stoi(q_ptr->sender_)] = opponent_pressed_button_symbol_;

            break;
        }
        case(16) : { // your_turn
            my_turn_.store(true);
            left_column_text_ = L"Your turn";
            break;
        }



        case(17) : { // you_lost {17,123,none}
            left_column_text_ = L"Ha ha you lost";
            set_defeat_cells_color(stoi(q_ptr->sender_));

            my_turn_.store(false);
            break;
        }

        case(18) : { // restart {18,none,none}
        // dialog window with restart proposal
            reset_game_field();
            break;
        }

        case(19) : { // tie
        // dialog window with restart proposal
            left_column_text_ = L"Tie!";
            my_turn_.store(false);
            break;
        }
    }
}


void drawing_device::set_title(const wstring& txt){
    upper_text_ = txt;
};




void drawing_device::send(const Query& q){
    queries_to_send_->push(q);
};


// Too lazy now to write a function that will update the vector partly
void drawing_device::remove_room_by_id(const string& r_id){

    for (auto& [number, room] : rooms_map_){

        if (room->id_ == r_id){
            int pos = number;
            rooms_map_.erase(pos); // erase the pair from the map

            for (auto iter = rooms_map_.find(pos++); iter != rooms_map_.end(); ++iter)
                *iter--; // update the rest


            hosted_rooms_.clear();
            break;
        }
    }

    for (auto& [number, room] : rooms_map_){
        hosted_rooms_.push_back(to_wstring(number) + L". " + room->players_);
    }

};


void drawing_device::reset_game_field(){

//    my_turn_.store(false);
    draw_fight();

    field_.init();

    for (auto& symb : symbols_)
        symb = not_pressed_button_symbol_;

    for (auto& cell : cell_colors_)
        cell = cells_color_;

    turns_done_ = 0;

};


void drawing_device::set_victory_cells_color(const int cells_numbers){
    // example won_ = 123
    cell_colors_[cells_numbers % 10] = victory_cell_color_; // 1
    cell_colors_[cells_numbers / 10 % 10] = victory_cell_color_; // 2
    cell_colors_[cells_numbers / 100 % 10] = victory_cell_color_; // 3
};


void drawing_device::set_defeat_cells_color(const int cells_numbers){
    // example won_ = 123
    cell_colors_[cells_numbers % 10] = defeat_cell_color_; // 1
    cell_colors_[cells_numbers / 10 % 10] = defeat_cell_color_; // 2
    cell_colors_[cells_numbers / 100 % 10] = defeat_cell_color_; // 3
};


void drawing_device::set_nickname(const string& nickname){
    nickname_ = to_wstring(nickname);
    send(Query("2,"+to_string(new_name_) + ",none"));
};










drawing_device::Chat_msg::Chat_msg(string& msg){
    auto it = find(msg.begin(), msg.end(), ':');
    sender_ = wstring(msg.begin(), it);
    text_ = wstring(it, msg.end());
}



drawing_device::Chat_msg::Chat_msg(const wstring& sender, const wstring& text) :
                                sender_(sender), text_(text) {};


drawing_device::Chat_msg::Chat_msg(const string& sender, const string& text) {
    sender_ = wstring(sender.begin(), sender.end());
    text_ = wstring(text.begin(), text.end());
};



bool drawing_device::Chat_msg::from(const wstring& sender) const{
    return sender == sender_;
};


shared_ptr<Node> drawing_device::Chat_msg::form_text(const alignment& align){
    switch (align) {
        case(alignment::right) : {
            return align_right(text(text_ + L" :" + sender_));
        }

        case(alignment::left) : {
            return text(sender_ + L": " + text_ );
        }
    }
};










//drawing_device::Room::Room(){

//}

drawing_device::Room::Room(const string& id, const wstring& players) :
                                    id_(id), players_(players){};



//drawing_device::Room::Room(const Room& rhs) : id_(rhs.id_), players_(rhs.players_){};

//drawing_device::Room& drawing_device::Room::operator=(const Room& rhs){
//    id_ = rhs.id_;
//    players_ = rhs.players_;
//    return *this;
//};




drawing_device::Game_Field::Game_Field(){
    init();
}


int drawing_device::won(){

    if (turns_done_ > 2){
        if (field_.diag_row_1())
            return 246;

        if (field_.diag_row_2())
            return 840;

        if (field_.hor_row())
            return 345;

        if (field_.vert_row())
            return 147;

        if (field_.top_row())
            return 210;

        if (field_.bottom_row())
            return 678;

        if (field_.left_row())
            return 630;

        if (field_.right_row())
            return 258;
    }

    return 0;
};


bool drawing_device::Game_Field::top_row() const {
    return cells_[0] && cells_[1] && cells_[2];
};


bool drawing_device::Game_Field::left_row() const{
    return cells_[0] && cells_[3] &&  cells_[6];
};


bool drawing_device::Game_Field::right_row() const{
    return cells_[2] && cells_[5] &&  cells_[8];
};


bool drawing_device::Game_Field::bottom_row() const{
    return  cells_[6] &&  cells_[7] &&  cells_[8];
};


bool drawing_device::Game_Field::hor_row() const{
    return cells_[3] && cells_[4] && cells_[5];
};


bool drawing_device::Game_Field::vert_row() const{
    return cells_[1] && cells_[4] &&  cells_[7];
};


bool drawing_device::Game_Field::diag_row_1() const{
    return cells_[2] && cells_[4] &&  cells_[6];
};


bool drawing_device::Game_Field::diag_row_2() const{
    return cells_[0] && cells_[4] &&  cells_[8];
};


void drawing_device::Game_Field::take_cell(const int& index){
    cells_[index] = true;
};


void drawing_device::Game_Field::init(){
    for (auto& cell : cells_)
        cell = false;
};























































