#include "client.h"

using namespace std;



int main(){



    try {


        boost::asio::io_context io_context;

        client user(io_context, "127.0.0.1", "6666");
        user.game_loop();

        boost::system::error_code error;
        if (error == boost::asio::error::eof)
            service_msg(msg_type::DEBUG, "connection closed by server");

        else if (error)
            throw boost::system::system_error(error);
        }


    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }




    return 0;
}
