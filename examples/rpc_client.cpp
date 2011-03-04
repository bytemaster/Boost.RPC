#include <boost/rpc/client.hpp>
#include <boost/utility.hpp>
#include <boost/rpc/tcp_connection.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/rpc/protocol_buffer.hpp>
#include <boost/idl/fast_delegate.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <iostream>

int handle_result( const std::string& r, int err )
{
    std::cerr << "Reply '"<<r<<"'  err: " << err << "\n";
    return 0;
}

int main( int argc, char** argv )
{
    using namespace boost;
    using namespace boost::asio;

    asio::io_service      io;
    boost::shared_ptr<ip::tcp::socket>  sock( new ip::tcp::socket(io) );
    sock->connect( ip::tcp::endpoint(ip::address::from_string(argv[1]), 
                                     lexical_cast<uint16_t>(argv[2]) ) );

    rpc::tcp::connection<rpc::protocol_buffer::protocol>::ptr 
        m_con_ptr( new rpc::tcp::connection<rpc::protocol_buffer::protocol>( sock ) );

    rpc::client<rpc::protocol_buffer::protocol>  m_client( m_con_ptr );

    m_client.call<std::string>( "ping", &handle_result );
    m_client.call<std::string>( "ping", &handle_result, boost::fusion::make_vector( std::string( "hello world" ) ) );

    new boost::asio::io_service::work(io);
    io.run(); 
    return 0;
}


