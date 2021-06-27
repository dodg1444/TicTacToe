#include "client.h"

using namespace std;



int main(int argc, const char* argv[]){

    // default
    string hostname = "127.0.0.1";
    string port = "6666";
    string nickname = "Default";

    if (argc == 4){
        hostname = argv[1];
        port = argv[2];
        nickname = argv[3];
    }

    try {

        boost::asio::io_context io_context;

        client game_client(io_context, hostname, port);
        game_client.init(nickname);

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
