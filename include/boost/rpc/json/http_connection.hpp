#ifndef _BOOST_RPC_JSON_HTTP_CONNECTION_HPP_
#define _BOOST_RPC_JSON_HTTP_CONNECTION_HPP_

namespace boost { namespace rpc { namespace json {
  
  
  /**
   *  Unlike the TCP connection, the HTTP connection will open a new
   *  connection for each call if necessary.  
   */
  class http_connection : public boost::rpc::json::connection {
    public:
      typedef boost::shared_ptr<http_connection> ptr;

      http_connection( const cmt::asio::tcp::socket::ptr& sock, 
                       cmt::thread* t = cmt::thread::current() );

      http_connection( const std::string& host, uint16_t port, cmt::thread* t = &cmt::thread::current() );

      void set_method( const std::string& method ); // POST / GET 
      void set_path( const boost::filesystem::path& p );
      void set_header( const std::string key, const std::string& value );
      boost::optional<std::string> get_header( const std::string& key );

    private:

  };

} } }

#endif
