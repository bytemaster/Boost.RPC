#include <boost/cmt/thread.hpp>
#include <boost/rpc/json/tcp/server.hpp>
#include <json_spirit/json_spirit_stream_reader.h>
#include <json_spirit/json_spirit_writer.h>

namespace js = json_spirit;
void handle_json( const js::Value& v ) {
    slog( "received" );
    json_spirit::write(v,std::cerr);
}

void on_recv( const boost::rpc::json::tcp::connection::ptr& c, const js::Value& v ) {
    try {
        std::cerr<<"server:";
        json_spirit::write(v,std::cerr);
        std::cerr<<std::endl;
        c->send(v);
    } catch ( const boost::exception& e  ) {
        elog( "%1%", boost::diagnostic_information(e) );
    }
}

void on_connection( const boost::rpc::json::tcp::connection::ptr& c ) {
    try {
        slog( "new connection" );
        c->set_recv_handler( boost::bind( on_recv, c, _1 ) );
    } catch ( const boost::exception& e  ) {
        elog( "%1%", boost::diagnostic_information(e) );
    }
}

int async_main( int argc, char** argv ) {
    if( argc <= 1 )
    {
        std::cerr << "Usage: jsons PORT\n";
        return -1;
    }
    using namespace boost;
    try {
        boost::rpc::json::tcp::listen( atoi(argv[1]), on_connection );
    } catch ( const boost::exception& e )
    {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
    return 0;
}

int main( int argc, char** argv )
{
    boost::cmt::async( boost::bind(async_main, argc, argv) );
    boost::cmt::exec();
    return 0;
}
