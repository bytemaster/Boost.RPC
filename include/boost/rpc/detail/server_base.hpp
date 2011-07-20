#ifndef _BOOST_RPC_DETAIL_SERVER_BASE_HPP_
#define _BOOST_RPC_DETAIL_SERVER_BASE_HPP_
#include <boost/signals.hpp>
#include <boost/asio.hpp>

namespace boost { namespace rpc { 
    typedef boost::asio::ip::tcp::socket  tcp_socket;
    typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;

    namespace detail {

    class server_base : public boost::enable_shared_from_this<server_base> {
        public:
            server_base();
            ~server_base();

            void listen( uint16_t port = 0 );

            // emited any time a new tcp connection is accepted
            boost::signals<void(const tcp_socket_ptr&)> new_connection;

        private:
            class server_base_private* my;
    };

} } } // boost::rpc::detail

#endif // _BOOST_RPC_DETAIL_SERVER_BASE_HPP_
