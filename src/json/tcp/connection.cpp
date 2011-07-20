#include <boost/rpc/json/tcp/connection.hpp>
#include <boost/json/reader_template.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {
    connection::connection( const connection::sock_ptr& p )
    :m_sock(p) {
    }

    bool connection::connect( const std::string& hostname, const std::string& port ) {
        m_sock = sock_ptr( new boost::cmt::asio::tcp::socket() );
        boost::system::error_code ec= m_sock->connect( boost::cmt::asio::tcp::resolve( hostname, port ).front() );
        if( !ec ) {
            elog( "%1%", boost::system::system_error(ec).what() );
        }
        return true;
    }

    void connection::send( const js::Value& v ) {
       std::stringstream ss;
       json_spirit::write(v,ss, json_spirit::remove_trailing_zeros);
       m_sock->write(ss.str().c_str(),ss.str().size());
    }

    void connection::set_recv_handler( const boost::function<void(const js::Value& v )>& v ) {
        m_recv_handler = v;
        async( boost::bind( &connection::read_loop, this ) );
    }

    void connection::read_loop() {
        try {
            js::Value v;
            boost::cmt::asio::tcp::socket::iterator itr(m_sock.get());
            boost::cmt::asio::tcp::socket::iterator end;
            std::string buf;

            int depth = 0;
            int quote = 0;
            while( itr != end ) {
                if( !quote ) {
                    if( *itr == '{' ) ++depth;
                    else if( *itr == '[' ) ++depth;
                    else if( *itr == '}' ) --depth;
                    else if( *itr == ']' ) --depth;
                    else if( *itr == '"' ) quote = 1;
                } else {
                    if( *itr == '"' ) quote = 0;
                }
                buf.push_back(*itr);
                if( depth == 0 ) {
                    json_spirit::read( buf, v );
                    m_recv_handler(v);
                    buf.resize(0);
                    v = js::Value();
                }
                ++itr;
            }
        } catch ( const std::exception& e ) {
            elog( "%1%", boost::diagnostic_information(e) );
            throw;
        }
    }
} } } }  // boost::rpc::json::tcp
