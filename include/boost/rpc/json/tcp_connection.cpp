#include <boost/rpc/json/tcp_connection.hpp>

namespace boost { namespace rpc { namespace json {

class tcp_connection_private {
  public:
      tcp_connection&               self;
      cmt::asio::tcp::socket::ptr   sock;
      boost::cmt::future<void>      read_done;

      tcp_connection_private( rpc::tcp_connection& s, const cmt::asio::tcp::socket::ptr& c ) 
      :self( s ), sock( c ) {
        read_done = self.get_thread()->async<void>( boost::bind(&tcp_connection_private::read_loop,this) );
      }

      ~tcp_connection_private() {
        sock->close();
        read_done.wait(); 
      }

      void read_loop() {
        // m_connected = true;
        try {
            json::value v;
            cmt::asio::tcp::socket::iterator itr(m_sock.get());
            cmt::asio::tcp::socket::iterator end;
            std::string buf;

            int depth = 0;
            int quote = 0;
            bool found = false;
            while( itr != end ) {
                if( !quote ) {
                    if( *itr == '{' ) ++depth;
                    else if( *itr == '[' ) ++depth;
                    else if( *itr == '}' ) --depth;
                    else if( *itr == ']' ) --depth;
                    else if( *itr == '"' ) quote = 1;

                    if( depth > 0 )
                      found = true;
                } else {
                    if( *itr == '"' ) quote = 0;
                }
                if( found )
                    buf.push_back(*itr);
                if( found && depth == 0 ) {
                    found = false;
                    //std::cerr<<"recv:";
                    json::read( buf, v );
                    //std::cerr<<buf<<std::endl;
                    on_receive(v);
                    buf.resize(0);
                    v = boost::rpc::json::value();
                }
                ++itr;
            }
        } catch ( const std::exception& e ) {
          elog( "%1%", boost::diagnostic_information(e) );
        }
        self.closed();
    } // read_loop

    // determine what kind of message it is and dispatch accordingly
    void on_receive( const json::value& v ) {
      if( v.contains("method") ) {
        if( v.contains( "id" ) ){
          self.handle_call(v);
        } else {
          self.handle_notice(v);
        }
      } else {
        if( v.contains( "id" ) ) {
          if( v.contains( "error" ) ) {
           self.handle_result(v);
          } else {
           self.handle_error(v);
          }
        } else {
           self.handle_error(v);
        }
      }
    }

};

tcp_connection::tcp_connection( const cmt::asio::tcp::socket::ptr& c, 
                                boost::cmt::thread* t ) 
:connection(t){
  my = new tcp_connection_private(*this, c );
}

tcp_connection::~tcp_connection() {
  delete my;
}

void tcp_connection::send( const json::value& msg ) {
    std::string str;
    json::to_string( msg, str); // TODO: write directly to socket instead of to temporary
    my->sock->write( str.c_str(), str.size() );
}

} } } // namespace boost::rpc::json
