#include <mace/rpc/json/client.hpp>
#include "cli.hpp"
#include <boost/exception/diagnostic_information.hpp>
#include <mace/rpc/json/http_client.hpp>
#include "bitcoin.hpp"

int main( int argc, char** argv ) {
 try {
   mace::rpc::json::http_client<bitcoin::client> bcd( argv[1] );

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
