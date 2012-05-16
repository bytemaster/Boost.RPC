#include <boost/rpc/json/client_base.hpp>
#include <boost/cmt/asio/tcp/socket.hpp>
#include <boost/rpc/json/tcp_connection.hpp>

namespace boost { namespace rpc { namespace json {
    client_base::client_base( const std::string& host, uint16_t port ) {
        m_con = boost::make_shared<rpc::json::tcp_connection>(host,port);
    }
}}} // boost::rpc::json
