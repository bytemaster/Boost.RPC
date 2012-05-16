#include <boost/network/protocol/http/client.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/rpc/json/http_client.hpp>
#include <boost/cmt/asio.hpp>

namespace boost { namespace rpc { namespace json {

// called from io_service 
void async_post_http_request( 
  const boost::rpc::json::connection::ptr& con,
  const boost::shared_ptr<boost::network::http::client>& c,
  const boost::shared_ptr<boost::network::http::client::request>& rq,
  const std::string& json_str,
  const std::string& req_id,
  const connection::pending_result::ptr& rs  = connection::pending_result::ptr() )
{
  try {
    boost::network::http::client::response resp = c->post(*rq,json_str);//"application/json",json_str);
    std::cout << boost::network::http::body(resp) << std::endl;
    if( rs ) {
      json::value v;
      json::from_string( boost::network::http::body(resp), v );
      if( req_id.size() && ( v.contains( "id" ) && std::string(v["id"]) != req_id) ) {
         rs->handle_error( 
            boost::copy_exception( boost::rpc::exception() 
                    << err_msg("Response ID did not match request") ) );
      }
    
      if( v.contains( "result" ) ) {
        rs->handle_result( *con, v["result"] );
      } else if( v.contains( "error" ) ) {
         json::value& err= v["error"];
         std::string msg = "";
         int64_t code = 0;
         std::string data = "";
         if( err.contains("message") ) msg = (std::string)err["message"];
         if( err.contains("code") ) code = (double)err["code"];
         if( err.contains("data") ) data = (std::string)err["data"];

         rs->handle_error( boost::copy_exception( 
                  boost::rpc::json::error_object() 
                               << json::err_code(code)
                               << json::err_data(data)
                               << rpc::err_msg(msg) ) );
      }
    }
  } catch( const boost::exception& e ) {
    if( rs ) rs->handle_error( boost::current_exception() );
  } catch ( ... ) {
    if( rs ) rs->handle_error( boost::current_exception() );
  }
}


/**
 *  Interfaces with boost::network::http::client to invoke methods and get the
 *  result async.
 */
class http_connection : public boost::rpc::json::connection {
  public:
      http_connection( const std::string& url )
      :m_client(new boost::network::http::client( boost::network::http::_cache_resolved = true)),m_url(url) {
        slog( "http connection %1%", this );
      }
      virtual void send( const json::value& msg, 
                         const connection::pending_result::ptr& pr ) {
        slog( "send %1% to %2%", json::to_string(msg), m_url );
        boost::shared_ptr<boost::network::http::client::request> req 
            = boost::make_shared<boost::network::http::client::request>(m_url);

        *req << boost::network::header("Authorization","Basic ");

       
        if( msg.contains( "id" ) ) {
           wlog( "id %1%", std::string( msg["id"]) );
        }
        std::string json_str;
        json::to_string( msg, json_str );
        boost::cmt::asio::default_io_service().post( 
          boost::bind( async_post_http_request, shared_from_this(),
                      m_client, req, json_str, std::string(msg["id"]), pr ) );
      }

      virtual void send( const json::value& msg ) {
        boost::shared_ptr<boost::network::http::client::request> req 
            = boost::make_shared<boost::network::http::client::request>(m_url);
        std::string json_str;
        json::to_string( msg, json_str );
        boost::cmt::asio::default_io_service().post( 
          boost::bind( async_post_http_request, 
                      shared_from_this(),
                      m_client, req, json_str, std::string(),
                      connection::pending_result::ptr() ) );
      }

      // thread that does the actual work... 
      std::string                                      m_url;
      boost::shared_ptr<boost::network::http::client>  m_client;
};


http_client_base::http_client_base( const std::string& url ) {
  client_base::m_con = boost::make_shared<http_connection>(url);
  slog( "m_con: %1%", m_con.get() );
}


} } } // boost::rpc::json
