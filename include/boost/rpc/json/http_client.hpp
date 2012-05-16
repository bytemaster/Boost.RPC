#ifndef _BOOST_RPC_JSON_HTTP_CLIENT_HPP_
#define _BOOST_RPC_JSON_HTTP_CLIENT_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/json/client_interface.hpp>
#include <boost/rpc/json/client_base.hpp>

namespace boost { namespace rpc {  namespace json {

  /**
   *  Sets client_base to a derived http_connection type which
   *  is implemented in cpp.
   */
  class http_client_base : public client_base {
    public:
      http_client_base( const std::string& url );
  };

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
                      public http_client_base {
    public:
      typedef boost::shared_ptr<http_client>   ptr;

      http_client( const std::string& url )
      :http_client_base(url) {
         boost::rpc::json::client_interface::set( *this );
      }

      http_client( const http_client& c ):http_client_base(c) {
        boost::rpc::json::client_interface::set( *this );
      }
      bool operator!()const { return !m_con; }

      http_client& operator=( const http_client& c ) {
        if( &c != this )  {
            m_con = c.m_con;
            boost::rpc::json::client_interface::set( *this );
        }
        return *this;
      }
  };

} } } // boost::rpc::json

#endif
