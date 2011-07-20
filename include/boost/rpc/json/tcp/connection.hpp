#ifndef _BOOST_RPC_JSON_TCP_CONNECTION_HPP_
#define _BOOST_RPC_JSON_TCP_CONNECTION_HPP_
#include <boost/rpc/json.hpp>
#include <boost/cmt/thread.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/cmt/asio/tcp/socket.hpp>
#include <boost/cmt/asio.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {

    class connection : public boost::rpc::json::connection {
        public:
            typedef boost::cmt::retainable_ptr<connection> ptr;
            typedef boost::cmt::asio::tcp::socket::ptr     sock_ptr;

            connection( const sock_ptr& s );
            connection(){}

            bool connect( const std::string& hostname, const std::string& port );

            void send( const js::Value& v );
            void set_recv_handler( const boost::function<void(const js::Value& v)>& recv );

        private:
            void read_loop();
            boost::function<void(const js::Value& v)> m_recv_handler;
            sock_ptr                                  m_sock;
    };

} } } } // boost::rpc::json::tcp

#endif
