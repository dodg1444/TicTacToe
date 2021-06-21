#pragma once

#include <string>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <map>
#include <chrono>

#include "tools.h"

#include "ftxui/component/checkbox.hpp"
#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Checkbox, Vertical
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/component/button.hpp"
#include "ftxui/component/menu.hpp"
#include "ftxui/component/input.hpp"
#include "ftxui/component/toggle.hpp"
#include <ftxui/dom/elements.hpp>




// replace with event handler : https://arthursonzogni.com/FTXUI/doc/_2examples_2util_2print_key_press_8cpp-example.html#a26
enum class game_stage {menu, settings, lobby, ingame};
// menu : lobby, settings, exit
// settings : change username, other customization
// lobby : join or host game
// ingame : game between players
// exit : leave the game


using namespace std;
using namespace ftxui;





class drawing_device{
public:
    drawing_device();

    void set_queue_ptr(const shared_ptr<threadsafe_q<Query>>);
    void process_query(shared_ptr<Query>);

    void draw_menu();
    void draw_settings();
    void draw_change_name();
    void draw_exit();
    void draw_host_game();
    void draw_join_game();
    void draw_fight();

    void event_handler();

    void init_ui();

    void set_title(const wstring&);

    void send(const Query&);

    void reset_game_field();

//    threadsafe_q<Query> queries_to_process_;

    class Chat_msg {
    public:
        enum class alignment {left, right};
        explicit Chat_msg(string&);
        Chat_msg(const wstring& sender, const wstring& text);
        Chat_msg(const string& sender, const string& text);
        wstring sender_;
        wstring text_;
        bool from(const wstring&) const;

        shared_ptr<Node> form_text(const alignment&);
    };


    class Room{
    public:
//        Room();
        Room(const string&, const wstring&);
//        Room(const Room&);
//        Room& operator=(const Room&);

        string id_;
        wstring players_;
    };

    void remove_room_by_id(const string&);


    class Game_Field{
    public:
        Game_Field();
        bool top_row() const;
        bool left_row() const;
        bool right_row() const;
        bool bottom_row() const;
        bool hor_row() const;
        bool vert_row() const;
        bool diag_row_1() const;
        bool diag_row_2() const;
        void take_cell(const int&);
        void init();
    private:
        bool cells_[9];
    };

    bool won();

private:
    wstring nickname_ = L"Default";
    shared_ptr<threadsafe_q<Query>> queries_to_send_;
    future<void> screen_loop_;
    future<void> screen_updater_;
    chrono::duration<float, ratio<1,1>> update_every_;


////////////////////////////////// MENU ///////////////////////////////////////////
    shared_ptr<ComponentBase> menu_;
    vector<wstring> menu_entries_names_;

////////////////////////////////// CHAT ///////////////////////////////////////////
    shared_ptr<ComponentBase> chat_components_;
    shared_ptr<ComponentBase> chat_input_;
    shared_ptr<ComponentBase> chat_printed_messages_;
    vector<Chat_msg> chat_messages_;
    wstring chat_msg_;
    Elements elems_; // queue<Element> elems_;
    size_t number_of_shown_messages_ = 12; // add to the settings



////////////////////////////////// CHANGE_NAME ////////////////////////////////////
    shared_ptr<ComponentBase> change_name_widget_;
    shared_ptr<ComponentBase> change_name_container_;
    shared_ptr<ComponentBase> change_name_tab_;
    wstring new_name_;


////////////////////////////////// EXIT ///////////////////////////////////////////
    shared_ptr<ComponentBase> exit_tab_;
    shared_ptr<ComponentBase> no_button_;
    shared_ptr<ComponentBase> yes_button_;
    shared_ptr<ComponentBase> exit_container_;
    wstring yes_button_text_;
    bool let_out_ = false;


//////////////////////////////// HOST GAME ////////////////////////////////////////

    shared_ptr<ComponentBase> host_game_container_;
    shared_ptr<ComponentBase> ready_toggle_;
    vector<wstring> toggle_entries_ = {L"Ready", L"Not ready"};
    int my_selection_;
    wstring opponent_status_ = L"Waiting...";
    wstring opponent_nickname_ = L"Waiting...";
    shared_ptr<ComponentBase> host_game_tab_;
    shared_ptr<ComponentBase> leave_game_button_;

//////////////////////////////// JOIN GAME ////////////////////////////////////////

    shared_ptr<ComponentBase> join_game_container_;
    shared_ptr<ComponentBase> rooms_list_;
    vector<wstring> hosted_rooms_;
    map<int, shared_ptr<Room>> rooms_map_; // [room_number : {123123, Joe}]
    int selected_room_;
    shared_ptr<ComponentBase> leave_lobby_button_;
    shared_ptr<ComponentBase> join_game_tab_;

////////////////////////////////// FIGHT //////////////////////////////////////////


    shared_ptr<ComponentBase> fight_container_;
    shared_ptr<ComponentBase> leave_fight_button_;
    shared_ptr<ComponentBase> restart_fight_button_;
    shared_ptr<ComponentBase> fight_tab_;
    wstring exit_button_text_;
    vector<shared_ptr<ComponentBase>> cells_; // buttons
    vector<wstring> symbols_; // symbols for each cell
    vector<int> ints_; // [0-8] ints for buttons to refer to
    wstring pressed_button_symbol_;
    wstring opponent_pressed_button_symbol_;
    wstring not_pressed_button_symbol_;
    atomic<bool> my_turn_;
    atomic<bool> in_fight_;
    int turns_done_;
    Game_Field field_;
    shared_ptr<ComponentBase> spinner_;
    int spinner_index_ = 0;
    atomic<bool> spin_;




////////////////////////////////// GITHUB /////////////////////////////////////////

    // open in browser




////////////////////////////////// MAIN ///////////////////////////////////////////
    game_stage stage_;

    ScreenInteractive screen_;
    Component container_;


    shared_ptr<ComponentBase> layout_;

    shared_ptr<ComponentBase> left_side_content_; // Component Tab
    int drawn_tab_number_ = 0;

    wstring upper_text_;
    wstring left_column_text_;

    int selected_entry_;




#if defined(_WIN32)
#endif
};
























