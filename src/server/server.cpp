//
// BIT HTTP Server
// Based on Boost.Asio
//

#include <server.hpp>
#include <router.hpp>

#include <utility>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/as_tuple.hpp>

#include <iostream>
#include <memory>
#include <string>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::dynamic_buffer;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::asio::as_tuple;

// ============================================================
// Client Session
// ============================================================

class BitSession :
    public std::enable_shared_from_this<BitSession>
{
private:
    tcp::socket socket_;

public:
    explicit BitSession(tcp::socket socket)
        : socket_(std::move(socket))
    {
        socket_.set_option(tcp::no_delay(true));
    }

    void start()
    {
        co_spawn(
            socket_.get_executor(),
            [self = shared_from_this()]
            {
                return self->handleRequest();
            },
            detached
        );
    }

private:
    awaitable<void> handleRequest()
    {
        try
        {
            constexpr std::size_t maxRequestSize = 8192;

            std::string request;

            co_await boost::asio::async_read_until(
                socket_,
                dynamic_buffer(request, maxRequestSize),
                "\r\n\r\n"
            );

            // Extract the HTTP request line.
            const std::size_t lineEnd =
                request.find("\r\n");

            if (lineEnd == std::string::npos)
            {
                co_return;
            }

            const std::string requestLine =
                request.substr(0, lineEnd);

            const std::size_t firstSpace =
                requestLine.find(' ');

            if (firstSpace == std::string::npos)
            {
                co_return;
            }

            const std::size_t secondSpace =
                requestLine.find(' ', firstSpace + 1);

            if (secondSpace == std::string::npos)
            {
                co_return;
            }

            const std::string method =
                requestLine.substr(0, firstSpace);

            std::string path =
                requestLine.substr(
                    firstSpace + 1,
                    secondSpace - firstSpace - 1
                );

            // This server currently handles GET requests.
            if (method != "GET")
            {
                co_return;
            }

            // Remove query parameters from the route path.
            const std::size_t queryPos = path.find('?');

            if (queryPos != std::string::npos)
            {
                path.erase(queryPos);
            }

            std::cout << "Request: "
                      << method << " "
                      << path << '\n';

            // Delegate URL handling to the router.
            const std::string response =
                routeRequest(path);

            co_await boost::asio::async_write(
                socket_,
                boost::asio::buffer(response)
            );

            boost::system::error_code ec;

            socket_.shutdown(
                tcp::socket::shutdown_both,
                ec
            );

            socket_.close(ec);
        }
        catch (const boost::system::system_error& e)
        {
            if (e.code() != boost::asio::error::eof &&
                e.code() != boost::asio::error::operation_aborted)
            {
                std::cerr << "Session error: "
                          << e.what() << '\n';
            }

            boost::system::error_code ec;
            socket_.close(ec);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Session error: "
                      << e.what() << '\n';

            boost::system::error_code ec;
            socket_.close(ec);
        }
    }
};

// ============================================================
// TCP Listener
// ============================================================

static awaitable<void> listen(tcp::acceptor& acceptor)
{
    for (;;)
    {
        auto [error, socket] =
            co_await acceptor.async_accept(as_tuple);

        if (!error)
        {
            boost::system::error_code ec;
            const auto remote = socket.remote_endpoint(ec);

            if (!ec)
            {
                std::cout << "Client connected: "
                          << remote << '\n';
            }

            std::make_shared<BitSession>(
                std::move(socket)
            )->start();
        }
        else
        {
            std::cerr << "Accept error: "
                      << error.message() << '\n';
        }
    }
}

// ============================================================
// Public Server Entry Point
// ============================================================

void startServer(
    const std::string& address,
    const std::string& port)
{
    io_context ctx;

    tcp::resolver resolver(ctx);

    const auto endpoints =
        resolver.resolve(
            address,
            port,
            tcp::resolver::passive
        );

    tcp::acceptor acceptor(ctx);

    acceptor.open(endpoints.begin()->endpoint().protocol());
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoints.begin()->endpoint());
    acceptor.listen();

    std::cout << "BIT Server started.\n"
              << "Listening on "
              << address << ":" << port << '\n';

    co_spawn(
        ctx,
        listen(acceptor),
        detached
    );

    ctx.run();
}
