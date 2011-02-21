#include <boost/rpc/protocol_buffer.hpp>
#include <boost/rpc/connection.hpp>
#include <boost/rpc/tcp_connection.hpp>
#include <boost/rpc/server.hpp>

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>


struct rpc_server
{
    rpc_server( boost::asio::io_service& ios, const boost::asio::ip::tcp::endpoint& ep )
    :a(ios, ep){}

    void start_accept()
    {
        boost::shared_ptr<boost::asio::ip::tcp::socket> s( new boost::asio::ip::tcp::socket( a.get_io_service() ) );
        a.async_accept( *s, boost::bind( &rpc_server::handle_accept, this, s, _1 ) );
    }

    void handle_accept( const boost::shared_ptr<boost::asio::ip::tcp::socket>& sock, const boost::system::error_code& ec  )
    {
        boost::rpc::tcp::connection<boost::rpc::protocol_buffer::protocol>::ptr 
            con_ptr( new boost::rpc::tcp::connection<boost::rpc::protocol_buffer::protocol>( sock ) );
        if( new_connection )
            new_connection( con_ptr );
        start_accept();
    }
    boost::function<void( const boost::rpc::tcp::connection<boost::rpc::protocol_buffer::protocol>::ptr) > new_connection;
    boost::asio::ip::tcp::acceptor   a;
};

int main( int argc, char** argv )
{
    elog( "Hello World" );
    using namespace boost;
    using namespace boost::asio;

    boost::asio::io_service      io;
    
    boost::rpc::server<rpc::protocol_buffer::protocol> rps;

    rpc_server serv( io, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),
                    lexical_cast<uint16_t>(argv[1])) );

    serv.start_accept();

    io.run();
    return 0;
}
