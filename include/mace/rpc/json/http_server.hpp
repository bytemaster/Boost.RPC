#ifndef _MACE_RPC_JSON_HTTP_SERVER_HPP_
#define _MACE_RPC_JSON_HTTP_SERVER_HPP_
#include <mace/stub/ptr.hpp>
#include <mace/rpc/json/connection.hpp>
#include <boost/any.hpp>

namespace mace { namespace rpc { namespace json { 

  /**
   *  Creates an HTTP server that will direct RPC requests to one or
   *  more services based upon the path.
   */
  class http_server {
    public:
      http_server( const std::string& ip, uint16_t port, uint16_t threads = 2 );
      ~http_server();

      template<typename InterfaceType, typename T>
      void add_service( const std::string& path, const boost::shared_ptr<T>& s );

      /**
       *  Add a new service and optionally associate an object with it that must share the
       *  lifetime of the service.  
       */
      void add_service( const std::string& path,  const rpc::json::connection::ptr&  c, const boost::any& s = boost::any() );

    private:
      class http_server_private* my;
  };


  template<typename InterfaceType, typename T>
  void http_server::add_service( const std::string& path, const boost::shared_ptr<T>& s ) {
    mace::stub::ptr<InterfaceType> session(s);
    json::connection::ptr con = boost::make_shared<rpc::json::connection>();
    mace::stub::visit( session, add_interface_visitor<InterfaceType>( *con, session ) );
    add_service( path, con, session );
  }

} } } // mace::rpc::json::http

#endif
