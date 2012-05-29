#include <mace/rpc/json/client_base.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <mace/rpc/json/tcp_connection.hpp>

namespace mace { namespace rpc { namespace json {
    client_base::client_base( const std::string& host, uint16_t port ) {
        m_con = boost::make_shared<rpc::json::tcp_connection>(host,port);
    }
}}} // mace::rpc::json
