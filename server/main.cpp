#include "tcp_server.h"




int main() {
    try {
        io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }

    catch (exception& exc) {
        cerr << exc.what() << "\n";
    }
    return 0;
}
