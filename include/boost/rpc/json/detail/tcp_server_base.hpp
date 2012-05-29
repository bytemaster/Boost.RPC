#ifndef _BOOST_RPC_JSON_TCP_SERVER_BASE_HPP_
#define _BOOST_RPC_JSON_TCP_SERVER_BASE_HPP_
#include <mace/cmt/thread.hpp>
#include <mace/stub/ptr.hpp>
#include <boost/fusion/support/deduce_sequence.hpp>
#include <boost/rpc/json/value_io.hpp>
#include <boost/rpc/json/connection.hpp>

namespace boost { namespace rpc { namespace json {
  
  namespace detail {
    class tcp_server_base {
      public:
        struct session_creator {
            virtual ~session_creator(){}
            virtual boost::any init_connection( const rpc::json::connection::ptr& ) = 0;
        };


        tcp_server_base( session_creator* sc, uint16_t port, mace::cmt::thread* t );
        ~tcp_server_base();
        mace::cmt::thread* get_thread()const;

      private:
        class tcp_server_base_private* my;
    };
  } // namesapce detal

} } } // boost::rpc::json

#endif // _BOOST_RPC_JSON_TCP_SERVER_BASE_HPP_
