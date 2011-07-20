#ifndef _BOOST_RPC_JSON_CONNECTION_HPP_
#define _BOOST_RPC_JSON_CONNECTION_HPP_
#include <boost/rpc/json.hpp>
#include <boost/cmt/retainable.hpp>
#include <boost/function.hpp>

namespace boost { namespace rpc { namespace json {

    class connection : public boost::cmt::retainable {
        public:
            typedef boost::cmt::retainable_ptr<connection>  ptr;
            typedef boost::function<void(const js::Value&)> recv_handler;

            virtual ~connection(){};
            virtual void send( const js::Value& v )=0;
            virtual void set_recv_handler( const recv_handler& recv )=0;
    };

} } } // boost::rpc::json

#endif// _BOOST_RPC_JSON_CONNECTION_HPP_
