#include <boost/rpc/json/tcp/connection.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {

    typedef boost::function<void(const boost::rpc::json::tcp::connection::ptr&)> handler;

    void listen( uint16_t port, const handler& handle );

} } } } // boost::rpc::json::tcp
