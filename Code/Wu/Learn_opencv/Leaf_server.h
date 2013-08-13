#ifndef YRUI_LEAF_SERVER_H_
#define YRUI_LEAF_SERVER_H_

#include <memory>
#include <boost/asio.hpp>

namespace Leaf { namespace Server {

using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::system::error_code;
namespace asio = boost::asio;

namespace Detail {

class Tcp_connection;

typedef std::shared_ptr<Tcp_connection> Connection_ptr;

}

/*!
 * \brief   Server.
 */
class Tcp_server {
public:
    Tcp_server();
    ~Tcp_server() = default;

    Tcp_server(const Tcp_server&) = delete;
    Tcp_server(Tcp_server&&) = delete;

    Tcp_server& operator=(const Tcp_server&) = delete;
    Tcp_server& operator=(Tcp_server&&) = delete;

    void run();

private:
    void start_listen_();

    void handle_accept_(
        Detail::Connection_ptr con, const error_code& ec
        );

private:
    io_service io_;
    tcp::acceptor acceptor_;
};

}}  // of namespace Leaf::Server

#endif // YRUI_LEAF_SERVER_H_
