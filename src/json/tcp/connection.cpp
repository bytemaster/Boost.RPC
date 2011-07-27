#include <boost/rpc/json/tcp/connection.hpp>
#include <boost/json/reader_template.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {
    connection::connection( const connection::sock_ptr& p )
    :m_sock(p) {
    }

    bool connection::is_connected()const { return m_connected; }

    bool connection::connect( const std::string& hostname, const std::string& port ) {
        m_sock = sock_ptr( new boost::cmt::asio::tcp::socket() );
        typedef std::vector<boost::asio::ip::tcp::endpoint> epvec;

        epvec eps = boost::cmt::asio::tcp::resolve( hostname, port );
        epvec::iterator itr = eps.begin();
        epvec::iterator end = eps.end();

        boost::system::error_code ec;
        do {
            std::cerr<< *itr<<std::endl;
            m_sock->close();
            ec = m_sock->connect( *itr );
            ++itr;
        } while( ec && itr != end );

        if( ec ) {
            elog( "%1%", boost::system::system_error(ec).what() );
            return false;
        }
       
        return true;
    }

    void connection::send( const boost::json::Value& v ) {
       std::stringstream ss;
       std::cerr<<"send:";
       boost::json::write(v,std::cerr, boost::json::remove_trailing_zeros);
       std::cerr<<"\n";
       boost::json::write(v,ss, boost::json::remove_trailing_zeros);
       m_sock->write(ss.str().c_str(),ss.str().size());
    }
    
    void connection::start() {
        async( boost::bind( &connection::read_loop, this ) );
    }

    void connection::read_loop() {
        m_connected = true;
        try {
            boost::json::Value v;
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
                    std::cerr<<"recv:";
                    boost::json::read( buf, v );
                    std::cerr<<buf<<std::endl;
                    boost::cmt::async( boost::bind(&connection::on_receive, this, v) );
                    buf.resize(0);
                    v = boost::json::Value();
                }
                ++itr;
            }
        } catch ( const std::exception& e ) {
            elog( "%1%", boost::diagnostic_information(e) );
            throw;
        }
        m_connected = false;
        disconnected();
    }
} } } }  // boost::rpc::json::tcp
