
#include <mace/rpc/json/client.hpp>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <mace/rpc/json/tcp_connection.hpp>
#include "calculator.hpp"

int main( int argc, char** argv ) {

  /*j
  std::vector<calc_str> batch_job(10);
  for( uint32_t i = 0; i < batch_job.size(); ++i ) {
    batch_job[i].left = i;
    batch_job[i].right = -i;
    batch_job[i].operation = "add";
  }
  */
  calc_str cstr;
  cstr.operation = "HElloWorld";
  std::vector<mace::cmt::future<double> > results(1000);
  try {
      mace::rpc::json::client<Calculator> c( argv[1], boost::lexical_cast<int>(argv[2]) );  
      boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
      double sum = 0;
      uint32_t i = 0;
      for( i = 0; i < 1000000; ++i ) {
         results[i%results.size()] = c->div(cstr);
         if( i%results.size() == results.size()-1 ) {
            for( uint32_t r = 0; r < results.size(); ++r ) {
              results[r].wait();
            }
         }
         if( i%10000 == 0 ) 
            std::cerr<<i<<"   "<<   i / ((boost::chrono::system_clock::now()-start).count() / (double)1000000000.0) << " c/s\n";
      }
      std::cerr<<   i / ((boost::chrono::system_clock::now()-start).count() / (double)1000000000.0) << " c/s\n";
  } catch ( const boost::exception& e ) {
    std::cerr<<boost::diagnostic_information(e)<<std::endl;
  } catch ( const std::exception& e ) {
    std::cerr<<boost::diagnostic_information(e)<<std::endl;
  }
}
