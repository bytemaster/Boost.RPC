#ifndef _BOOST_RPC_JSON_HTTP_CLIENT_HPP_
#define _BOOST_RPC_JSON_HTTP_CLIENT_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/json/client_interface.hpp>
#include <boost/rpc/json/client_base.hpp>
#include <boost/rpc/json/http_connection.hpp>

namespace boost { namespace rpc {  namespace json {

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType>
  class http_client : public boost::reflect::any_ptr<InterfaceType, boost::rpc::json::client_interface>, 
                      public client_base {
    public:
      typedef boost::shared_ptr<http_client>   ptr;

      http_client( const std::string& url )
      :client_base( boost::make_shared<http_connection>(url) ) {
         boost::rpc::json::client_interface::set( *this );
      }

      http_client( const http_client& c ):client_base(c) {
        boost::rpc::json::client_interface::set( *this );
      }

      bool operator!()const { return !m_con; }
  };

} } } // boost::rpc::json

#endif
