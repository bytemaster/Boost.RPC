#include <iostream>
#include <boost/rpc/json/client.hpp>
#include "calculator.hpp"
#include "cli.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/rpc/json/http_client.hpp>
#include "bitcoin.hpp"



int main( int argc, char** argv ) {
 try {
   boost::rpc::json::http_client<bitcoin::client> bcd( argv[1] );
//   boost::reflect::any_ptr<bitcoin::client> s = bcd;

   cli  m_cli(bcd);
   m_cli.start();

   mace::cmt::exec();
 } catch (const  boost::exception& e ) {
   elog( "%1%", boost::diagnostic_information(e) );
 } catch (const  std::exception& e ) {
   elog( "%1%", boost::diagnostic_information(e) );
 } catch ( ... ) {
   elog( "Caught unknown exception" );
 }
  return 0;
}
