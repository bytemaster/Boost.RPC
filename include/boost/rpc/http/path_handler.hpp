#ifndef _BOOST_RPC_HTTP_PATH_HANDLER_HPP_
#define _BOOST_RPC_HTTP_PATH_HANDLER_HPP_
#include <boost/rpc/http/request.hpp>
#include <boost/rpc/http/reply.hpp>
#include <boost/function.hpp>
#include <map>
#include <string>

namespace boost { namespace rpc { namespace http {
  
  class path_handler {
    public:
        typedef boost::function<bool(const request&,const std::string&,reply&)> handler_type;

    /// Handle a request and produce a reply.
        void operator()(const request& req, reply& rep);
    
        void register_handler( const std::string& prefix, const handler_type& h );

    private:
       std::map<std::string,handler_type> m_handlers;
  };

}}} // boost::rpc::http
#endif

