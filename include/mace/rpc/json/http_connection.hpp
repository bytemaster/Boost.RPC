#ifndef _MACE_RPC_JSON_HTTP_CONNECTION_HPP_
#define _MACE_RPC_JSON_HTTP_CONNECTION_HPP_
#include <mace/rpc/json/connection.hpp>

namespace mace { namespace rpc { namespace json {
  
  /**
   *  Unlike the TCP connection, the HTTP connection will open a new
   *  connection for each call if necessary.  
   */
  class http_connection : public mace::rpc::json::connection {
    public:
      enum http_method { POST, GET };
      typedef boost::shared_ptr<http_connection> ptr;

      http_connection( const std::string& url = "" );
      ~http_connection();

      void set_url( const std::string& url );
      void set_http_method( const std::string& method ); // POST / GET 
      void set_path( const std::string& path );
      void set_args( const std::string& args );
      void set_header( const std::string key, const std::string& value );
      boost::optional<std::string> get_header( const std::string& key );

    protected:
      virtual void send( const json::value& msg, const connection::pending_result::ptr& pr );
      virtual void send( const json::value& msg );

    private:
      class http_connection_private* my;

  };

} } } // mace::rpc::json

#endif
