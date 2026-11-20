
//
// BIT HTTP Server
// Application Entry Point
//

#include "server.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    try
    {
        // Check command-line arguments
        if (argc != 3)
        {
            std::cerr
                << "Usage: Bit <listen_address> <listen_port>\n"
                << "Example: Bit 0.0.0.0 2232\n";

            return 1;
        }

        const std::string address = argv[1];
        const std::string port = argv[2];

        // Start the BIT HTTP Server
        startServer(address, port);
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Server error: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}