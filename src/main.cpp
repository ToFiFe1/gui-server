//
// Bit HTTP Server
// Based on Boost.Asio
//

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <files/file_manager.hpp>
#include <api/Apis.hpp>


// implement my filemanager




using boost::asio::as_tuple;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::dynamic_buffer;
using boost::asio::io_context;
using boost::asio::ip::tcp;




 


// ============================================================
// Client Session
// ============================================================

class BitSession :
    public std::enable_shared_from_this<BitSession>
{
private:

    // TCP connection to the client
    tcp::socket socket_;

public:

    // Constructor
    BitSession(tcp::socket socket)
        : socket_(std::move(socket))
    {
        // Disable Nagle's algorithm.
        // Small packets can be sent immediately.
        socket_.set_option(tcp::no_delay(true));
    }


    // Start the session
    void start()
    {
        co_spawn(
            socket_.get_executor(),

            [self = shared_from_this()]
            {
                return self->handle_request();
            },

            detached
        );
    }


private:

    // ========================================================
    // Handle HTTP request
    // ========================================================

    awaitable<void> handle_request()
    {
        try
        {
            constexpr std::size_t max_request_size = 8192;

            std::string request;

            // ------------------------------------------------
            // Read HTTP request
            // ------------------------------------------------

            std::size_t length =
                co_await boost::asio::async_read_until(
                    socket_,
                    dynamic_buffer(request, max_request_size),
                    "\r\n\r\n"
                );


            // ------------------------------------------------
            // Debug output
            // ------------------------------------------------

            //std::cout << "\n========== HTTP REQUEST ==========\n";

            //std::cout << request.substr(0, length);

            //std::cout << "==================================\n";


            // ------------------------------------------------
            // HTML body
            // ------------------------------------------------

            
            
          
            // ------------------------------------------------
            // error
            //-------------------------------------------------
            std::size_t start = request.find(' ') + 1;
            std::size_t end = request.find(' ', start);

            std::string path = request.substr(start, end - start);
            std::cout << path << "\n";  
            // steht oben



            std::string response;

            

            if (path == "/")
            {               
                response = onload();
            }
            else
            {
                response = handleRequest(path);
                
            }




          

            // ------------------------------------------------
            // Send response
            // ------------------------------------------------

            std::cout << "Sending HTTP response...\n";

            co_await boost::asio::async_write(
                socket_,
                boost::asio::buffer(response)
            );


            std::cout << "Response sent.\n";
            

            // ------------------------------------------------
            // Close connection
            // ------------------------------------------------

            boost::system::error_code ec;

            socket_.shutdown(
                tcp::socket::shutdown_both,
                ec
            );

            socket_.close(ec);
        }
        catch (const boost::system::system_error& e)
{
    if (e.code() == boost::asio::error::eof)
    {
        std::cout << "Client closed connection.\n";
    }
    else
    {
        std::cerr
            << "Session error: "
            << e.what()
            << "\n";
    }

    boost::system::error_code ec;
    socket_.close(ec);
}
catch (const std::exception& e)
{
    std::cerr
        << "Session error: "
        << e.what()
        << "\n";

    boost::system::error_code ec;
    socket_.close(ec);
}
    }
};


// ============================================================
// TCP Listener
// ============================================================

awaitable<void> listen(tcp::acceptor& acceptor)
{
    for (;;)
    {
        // ----------------------------------------------------
        // Wait for a client
        // ----------------------------------------------------

        auto [error, socket] =
            co_await acceptor.async_accept(as_tuple);


        // ----------------------------------------------------
        // Check connection
        // ----------------------------------------------------

        if (!error)
        {
            std::cout
                << "Client connected: "
                << socket.remote_endpoint()
                << "\n";


            // ------------------------------------------------
            // Create a session for this client
            // ------------------------------------------------

            std::make_shared<BitSession>(
                std::move(socket)
            )->start();
        } 
        else
        {
            std::cerr
                << "Accept error: "
                << error.message()
                << "\n";
        }
    }
}


// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[])
{
    try
    {
        // ----------------------------------------------------
        // Check arguments
        // ----------------------------------------------------

        if (argc != 3)
        {
            std::cerr
                << "Usage: Bit.exe "
                << "<listen_address> "
                << "<listen_port>\n";

            std::cerr
                << "Example: "
                << "Bit.exe 192.168.1.48 2232\n";

            return 1;
        }


        // ----------------------------------------------------
        // Create Asio event loop
        // ----------------------------------------------------

        io_context ctx;
        


        // ----------------------------------------------------
        // Resolve IP + port
        // ----------------------------------------------------

        auto endpoint =
            *tcp::resolver(ctx)
                .resolve(
                    argv[1],
                    argv[2],
                    tcp::resolver::passive
                )
                .begin();


        // ----------------------------------------------------
        // Create TCP acceptor
        // ----------------------------------------------------

        tcp::acceptor acceptor(
            ctx,
            endpoint
        );


        // ----------------------------------------------------
        // Start listener
        // ----------------------------------------------------

        std::cout
            << "Bit Server started.\n";

        std::cout
            << "Listening on "
            << argv[1]
            << ":"
            << argv[2]
            << "\n";


        co_spawn(
            ctx,
            listen(acceptor),
            detached
        );


        // ----------------------------------------------------
        // Run Asio
        // ----------------------------------------------------

        ctx.run();
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Server error: "
            << e.what()
            << "\n";

        return 1;
    }

    return 0;
}   