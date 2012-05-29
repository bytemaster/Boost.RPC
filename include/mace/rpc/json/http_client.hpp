#ifndef _MACE_RPC_JSON_HTTP_CLIENT_HPP_
#define _MACE_RPC_JSON_HTTP_CLIENT_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/json/client_interface.hpp>
#include <mace/rpc/json/client_base.hpp>
#include <mace/rpc/json/http_connection.hpp>

namespace mace { namespace rpc {  namespace json {

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType>
  class http_client : public mace::stub::ptr<InterfaceType, mace::rpc::json::client_interface>, 
                      public client_base {
    public:
      typedef boost::shared_ptr<http_client>   ptr;

      http_client( const std::string& url )
      :client_base( boost::make_shared<http_connection>(url) ) {
         mace::rpc::json::client_interface::set( *this );
      }

      http_client( const http_client& c ):client_base(c) {
        mace::rpc::json::client_interface::set( *this );
      }

      bool operator!()const { return !m_con; }
  };

} } } // mace::rpc::json

#endif
