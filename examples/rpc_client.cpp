#include <iostream>
#include <boost/rpc/json/client.hpp>
#include "calculator.hpp"
#include "cli.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>


int main2( int argc, char** argv ) {
    if( argc <= 2 ) {
        std::cerr << "Usage: "<<argv[0]<<" IP PORT\n";
        return 0;
    }
    try {
      boost::rpc::json::client<Calculator> calc( argv[1], boost::lexical_cast<int>(argv[2]) );  
      std::cerr<<"Result: "<<(double)calc->add(5.55)<<std::endl;
      std::cerr<<"Result: "<<(double)calc->add(5.55)<<std::endl;
      calc->add.notice(5.55);
      std::cerr<<"Result: "<<(double)calc->result()<<std::endl;

      std::cerr<< calc->npt( named_param_test( 1 ) ) <<std::endl;
      std::cerr<< calc->npt( named_param_test( 1,2 ) ) <<std::endl;
      std::cerr<< calc->npt( named_param_test() ) <<std::endl;


    } catch ( const boost::exception& e ) {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
}

int main( int argc, char** argv ) {
  boost::cmt::async( boost::bind(main2,argc,argv) );
  boost::cmt::exec();
  return 0;
}

