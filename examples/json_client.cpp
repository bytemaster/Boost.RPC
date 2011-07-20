#include <boost/cmt/thread.hpp>
#include <boost/rpc/json/tcp/connection.hpp>
#include <json_spirit/json_spirit_stream_reader.h>
#include <json_spirit/json_spirit_writer.h>

namespace js = json_spirit;
void handle_json( const js::Value& v ) {
    slog( "received" );
    json_spirit::write(v,std::cerr);
}

int async_main( int argc, char** argv ) {
    if( argc <= 2 )
    {
        std::cerr << "Usage: json_client HOST PORT\n";
        return -1;
    }
    using namespace boost;
    try {
        boost::rpc::json::tcp::connection* con = new boost::rpc::json::tcp::connection(); 
        if( !con->connect( argv[1], argv[2] ) ) {
            elog( "Unable to connect to %1%:%2%", argv[1], argv[2] );
            return 0;
        }
        slog( "connected" );
        con->set_recv_handler(handle_json);

        boost::cmt::thread* read_thread = boost::cmt::thread::create();
        js::Value v;
        js::Stream_reader<std::istream, js::Value> reader(std::cin);
        while( read_thread->sync<bool>( 
               boost::bind(&js::Stream_reader<std::istream,js::Value>::read_next, &reader, boost::ref(v) )  ) ) {
            con->send(v);
            v = js::Value();
        }

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
