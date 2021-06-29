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

enum class game_stage {menu, settings, lobby, ingame};

using namespace std;
using namespace ftxui;

class drawing_device{
public:
    drawing_device();
    drawing_device(const drawing_device&) = delete;
    drawing_device& operator=(const drawing_device&) = delete;
    void set_queue_ptr(const shared_ptr<threadsafe_q<Query>>);
    void process_query(shared_ptr<Query>);
    void draw_menu();
    void draw_settings();
    void draw_change_name();
    void draw_exit();
    void draw_host_game();
    void draw_join_game();
    void draw_fight();
    void set_nickname(const string&);
    void event_handler();
    void init_ui();
    void set_title(const wstring&);
    void send(const Query&);
    void reset_game_field();

    struct Chat_msg {
        enum class alignment {left, right};
        explicit Chat_msg(string&);
        Chat_msg(const wstring& sender, const wstring& text);
        Chat_msg(const string& sender, const string& text);
        Chat_msg(const Chat_msg&) = delete;
        Chat_msg& operator=(const Chat_msg&) = delete;
        Chat_msg(Chat_msg&&) = default;
        Chat_msg& operator=(Chat_msg&&) = default;
        bool from(const wstring&) const;
        shared_ptr<Node> form_text(const alignment&);

        wstring sender_;
        wstring text_;
    };

    struct Room{
        Room(const string&, const wstring&);
        Room(const Room&) = delete;
        Room& operator=(const Room&) = delete;

        string id_;
        wstring players_;
    };

    void remove_room_by_id(const string&);

    class Game_Field{
    public:
        Game_Field();
        Game_Field(const Game_Field&) = delete;
        Game_Field& operator=(const Game_Field&) = delete;
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

    int won();
    void set_victory_cells_color(const int);
    void set_defeat_cells_color(const int);

private:
    wstring nickname_ = L"Default";
    shared_ptr<threadsafe_q<Query>> queries_to_send_;
    future<void> screen_loop_;
    future<void> screen_updater_;
    chrono::duration<float, ratio<1,1>> update_every_;

////////////////////////////////// MENU ///////////////////////////////////////////
    Component menu_;
    vector<wstring> menu_entries_names_;

////////////////////////////////// CHAT ///////////////////////////////////////////
    Component chat_components_;
    Component chat_input_;
    Component chat_printed_messages_;
    Component clear_chat_button_;
    vector<Chat_msg> chat_messages_;
    wstring chat_msg_;
    Elements elems_; // queue<Element> elems_;
    size_t number_of_shown_messages_ = 24; // add to the settings

////////////////////////////////// CHANGE_NAME ////////////////////////////////////
    Component change_name_widget_;
    Component change_name_container_;
    Component change_name_tab_;
    wstring new_name_;


////////////////////////////////// EXIT ///////////////////////////////////////////
    Component exit_tab_;
    Component no_button_;
    Component yes_button_;
    Component exit_container_;
    wstring yes_button_text_;
    bool let_out_ = false;

//////////////////////////////// HOST GAME ////////////////////////////////////////
    Component host_game_container_;
    Component ready_toggle_;
    vector<wstring> toggle_entries_ = {L"Ready", L"Not ready"};
    int my_selection_;
    wstring opponent_status_ = L"Waiting...";
    wstring opponent_nickname_ = L"Waiting...";
    Component host_game_tab_;
    Component leave_game_button_;
    Color ready_color_ = Color::DarkRed;
    Color opponent_ready_color_ = Color::DarkRed;

//////////////////////////////// JOIN GAME ////////////////////////////////////////
    Component join_game_container_;
    Component rooms_list_;
    vector<wstring> hosted_rooms_;
    map<int, shared_ptr<Room>> rooms_map_; // [room_number : {123123, Joe}]
    int selected_room_;
    Component leave_lobby_button_;
    Component join_game_tab_;

////////////////////////////////// FIGHT //////////////////////////////////////////
    Component fight_container_;
    Component leave_fight_button_;
    Component restart_fight_button_;
    Component fight_tab_;
    wstring exit_button_text_;
    vector<Component> cells_; // buttons
    vector<wstring> symbols_; // symbols for each cell
    vector<int> ints_; // [0-8] ints for buttons to refer to
    vector<Color> cell_colors_; // own color for each cell
    wstring pressed_button_symbol_;
    wstring opponent_pressed_button_symbol_;
    wstring not_pressed_button_symbol_;
    atomic<bool> my_turn_;
    atomic<bool> in_fight_;
    int turns_done_;
    Game_Field field_;
    Component spinner_;
    int spinner_index_ = 0;
    atomic<bool> spin_;

////////////////////////////////// GITHUB /////////////////////////////////////////
    // open in browser

////////////////////////////////// MAIN ///////////////////////////////////////////
    game_stage stage_;
    Color main_color_ = Color::DarkMagenta;
    Color cells_color_ = Color::DarkCyan;
    Color victory_cell_color_ = Color::DarkGreen;
    Color defeat_cell_color_ = Color::DarkRed;
    ScreenInteractive screen_;
    Component container_;
    Component layout_;
    Component left_side_content_; // Component Tab
    int drawn_tab_number_ = 0;
    wstring upper_text_;
    wstring left_column_text_;
    int selected_entry_;
};
