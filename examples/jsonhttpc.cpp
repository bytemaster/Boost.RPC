#include <iostream>
#include <boost/rpc/json/client.hpp>
#include "calculator.hpp"
#include "cli.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/rpc/json/http_client.hpp>

int main( int argc, char** argv ) {
 try {
   boost::rpc::json::http_client<Calculator> client( argv[1] );
   slog( "test add" );
   std::cerr<<(double)client->add(3.131592);
   slog( "another add" );
   std::cerr<<(double)client->add(3.131592);
   slog( "exiting" );
 } catch (const  boost::exception& e ) {
   elog( "%1%", boost::diagnostic_information(e) );
 } catch (const  std::exception& e ) {
   elog( "%1%", boost::diagnostic_information(e) );
 } catch ( ... ) {
   elog( "Caught unknown exception" );
 }
  return 0;
}
