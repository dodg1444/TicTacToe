#include "tcp_server.h"




int main(int argc, char** argv) {

    int port;

    if (argc == 1 || argc > 2)
        port = 6666;

    else
        port = atoi(argv[1]);



    try {
        io_context io_context;
        tcp_server server(io_context, port);
        io_context.run();
    }

    catch (exception& exc) {
        cerr << exc.what() << "\n";
    }
    return 0;
}
