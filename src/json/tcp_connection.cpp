#include <mace/cmt/asio.hpp>
#include <mace/rpc/error.hpp>
#include <mace/rpc/json/tcp_connection.hpp>

namespace mace { namespace rpc { namespace json {

class tcp_connection_private {
  public:
      tcp_connection&                     self;
      mace::cmt::asio::tcp::socket::ptr   sock;
      mace::cmt::future<void>            read_done;

      tcp_connection_private( rpc::json::tcp_connection& s, const mace::cmt::asio::tcp::socket::ptr& c ) 
      :self( s ), sock( c ) {
        read_done = self.get_thread()->async<void>( boost::bind(&tcp_connection_private::read_loop,this) );
      }

      ~tcp_connection_private() {
        sock->close();
        read_done.wait(); 
      }

      /// @todo break all promises when connection is closed.
      void read_loop() {
        // m_connected = true;
        try {
            json::value v;
            mace::cmt::asio::tcp::socket::iterator itr(sock.get());
            mace::cmt::asio::tcp::socket::iterator end;
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
                  //  std::cerr<<"recv '"<<buf<<"'\n";
                    json::from_string( buf, v );
                    //std::cerr<<buf<<std::endl;
                    mace::cmt::async( boost::bind(&tcp_connection_private::on_receive, this, v ) );
                    buf.resize(0);
                    v = mace::rpc::json::value();
                }
                ++itr;
            }
        } catch ( const std::exception& e ) {
          //wlog( "%1%", boost::diagnostic_information(e) );
        }
        self.closed();
    } // read_loop

    // determine what kind of message it is and dispatch accordingly
    void on_receive( const json::value& v ) {
      if( v.contains("method") ) {
        if( v.contains( "id" ) ){
          if( v["id"] == json::null_t() )
            self.handle_notice(v);
          else
            self.handle_call(v);
          return;
        } else {
          self.handle_notice(v);
          return;
        }
      } else {
        if( v.contains( "id" ) ) {
          if( v.contains( "result" ) ) {
            self.handle_result(v);
            return;
          } 
        }
      }
      self.handle_error(v);
    }

};

tcp_connection::tcp_connection( const mace::cmt::asio::tcp::socket::ptr& c, 
                                mace::cmt::thread* t ) 
:connection(t){
  my = new tcp_connection_private(*this, c );
}


tcp_connection::tcp_connection( const std::string& host, uint16_t port,
                                mace::cmt::thread* t ) 
:connection(t){
  std::vector<boost::asio::ip::tcp::endpoint> eps = mace::cmt::asio::tcp::resolve(host, boost::lexical_cast<std::string>(port) );
  for( uint32_t i = 0; i < eps.size(); ++i ) {
    mace::cmt::asio::tcp::socket::ptr sock = boost::make_shared<mace::cmt::asio::tcp::socket>();
    if( !sock->connect(eps[i]) ) {
      my = new tcp_connection_private(*this, sock );
      return;
    } 
  }
  MACE_RPC_THROW( "Unable to connect to host %1%:%2%", %host %port );
}


tcp_connection::~tcp_connection() {
  delete my;
}

void tcp_connection::send( const json::value& msg ) {
    std::string str;
    json::to_string( msg, str); // TODO: write directly to socket instead of to temporary
 //   std::cerr<<"send:"<<str<<std::endl;
    my->sock->write( str.c_str(), str.size() );
}

} } } // namespace mace::rpc::json
