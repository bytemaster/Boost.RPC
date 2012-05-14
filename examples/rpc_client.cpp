#include <iostream>
#include <boost/rpc/json/client.hpp>
#include "calculator.hpp"
#include "cli.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/rpc/json/tcp_connection.hpp>


int main( int argc, char** argv ) {
    if( argc <= 2 ) {
        std::cerr << "Usage: "<<argv[0]<<" IP PORT\n";
        return 0;
    }
    try {
      boost::rpc::json::client<Calculator> calc( argv[1], boost::lexical_cast<int>(argv[2]) );  
      std::cerr<<"Result: "<<(double)calc->add(5.55)<<std::endl;
      std::cerr<<"Result: "<<(double)calc->add(5.55)<<std::endl;

      calc->add(3.1415); // TODO: Automatically convert to a notice...

      calc->add.notice(5.55);
      std::cerr<<"Result: "<<(double)calc->result()<<std::endl;

      std::cerr<< calc->npt( named_param_test( 1 ) ) <<std::endl;
      std::cerr<< calc->npt( named_param_test( 1,2 ) ) <<std::endl;
      std::cerr<< calc->npt( named_param_test() ) <<std::endl;

      boost::rpc::json::tcp_connection con( argv[1], boost::lexical_cast<int>(argv[2]) );

      boost::rpc::json::value r = con.call( "add", 6.666 );
      /*
      json::value r = con->call( "add", rpc::named_params()
                                          ("one", 3.45)
                                          ("two", "paramtwo")
                                          );
      */
      con.notice_fused( "add", boost::fusion::make_vector(6.66) );

      std::cerr<<"Result: "<<(double)con.call( "result" ).wait()<<std::endl;

    } catch ( const boost::exception& e ) {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
    return 0;
}

int main2( int argc, char** argv ) {
  boost::cmt::async( boost::bind(main2,argc,argv) );
  boost::cmt::exec();
  return 0;
}

