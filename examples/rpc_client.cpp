#include "calculator.hpp"
#include <boost/rpc/client.hpp>
#include "cli.hpp"
#include <boost/cmt/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/rpc/json/client.hpp>
#include <boost/rpc/json/tcp/connection.hpp>


/**
 *  This is a simple RPC client that will connect to an
 *  RPC server that implements the Calculator interface.
 *
 *  In this case, the actual rpc interface is also wrapped in the
 *  command line interface for Calculator.  So the flow of control
 *  becomes:
 *
 *  User    -> CLI -> RPC Client -> RPC Server -> Implementation
 *  Display <- CLI <- RPC Client <- RPC Server <- 
 *
 *
 */
void amain(int argc, char**argv );
int main( int argc, char** argv )
{
    boost::cmt::async( boost::bind( amain, argc, argv ) );
    return boost::cmt::exec();
}
void amain(int argc, char**argv ) {

    if( argc <= 2 )
    {
        std::cerr << "Usage: rpc_client IP PORT\n";
        return;
    }
    using namespace boost;
    try {
        rpc::json::tcp::connection::ptr con( new boost::rpc::json::tcp::connection() );
        if( !con->connect( argv[1], argv[2] ) ) {
            std::cerr<<"Error connecting to "<<argv[1]<<":"<<argv[2]<<"\n"; 
            return;
        }
        rpc::json::client<Calculator>::ptr calc(new rpc::json::client<Calculator>(con));
        
        reflect::any<Calculator> s;
        s= *calc;
        cli  m_cli;
        m_cli.start_visit(s);

        std::string line;
        std::string cmd;
        std::string args;
        boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
        double sum = 0;
        int i = 0;
        for( i = 0; i < 100000; ++i ) {
            sum += calc->add(5);
        }
        boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
        uint64_t us = (end-start).total_microseconds();
        std::cerr << i << " add(5) took  " << us << "us   " << double(i) / (us/1000000.0) << "invoke/sec\n";

        while( true )
        {
            std::cerr << "Enter Method: ";
            std::getline( std::cin, line );
            cmd = line.substr( 0, line.find('(') );
            args = line.substr( cmd.size(), line.size() );
            std::cerr << m_cli[cmd](args) << std::endl;
        }
    
    } catch ( const boost::exception& e )
    {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
}


