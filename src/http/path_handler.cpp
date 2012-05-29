#include <mace/rpc/http/path_handler.hpp>
#include <boost/lexical_cast.hpp>
#include <mace/rpc/http/mime_types.hpp>
#include <mace/rpc/http/reply.hpp>
#include <mace/rpc/http/request.hpp>
#include <mace/rpc/http/file_handler.hpp>

namespace mace { namespace rpc { namespace http {

 // TODO: make sure path_handler is thread safe... this handler may be called from any ADIO thread
void path_handler::operator()(const request& req, reply& rep) {
   // Decode url to path.
   std::string request_path;
   if (!url_decode(req.uri, request_path)) {
     rep = reply::stock_reply(reply::bad_request);
     return;
   }
   // Request path must be absolute and not contain "..".
   if (request_path.empty() || request_path[0] != '/'
       || request_path.find("..") != std::string::npos)
   {
     rep = reply::stock_reply(reply::bad_request);
     return;
   }
   std::string base_path = request_path.substr(0, request_path.find('?') );
   // TODO: make sure this is thread safe... this handler may be called from any ADIO thread
   std::map<std::string,handler_type>::const_iterator itr = m_handlers.begin();//m_handlers.lower_bound(request_path);   
   while( itr != m_handlers.end() ) {
     if( itr->first != base_path.substr(0,itr->first.size()) ) {
       //wlog( "stop search before %1% != %2% ", itr->first, base_path.substr(0,itr->first.size()) );
       break;
     }
     if(itr->second(req,request_path.substr(itr->first.size()),rep) ) {
       return;
     } 
     ++itr;
   } 
   rep = reply::stock_reply(reply::not_found);
   return;
}
void path_handler::register_handler( const std::string& prefix, const handler_type& h ) {
  if( !h ) m_handlers.erase(prefix);
  else m_handlers[prefix] = h;
}

} } } // mace::rpc::http
