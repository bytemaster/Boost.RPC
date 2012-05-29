#ifndef _MACE_RPC_HTTP_PATH_HANDLER_HPP_
#define _MACE_RPC_HTTP_PATH_HANDLER_HPP_
#include <mace/rpc/http/request.hpp>
#include <mace/rpc/http/reply.hpp>
#include <boost/function.hpp>
#include <map>
#include <string>

namespace mace { namespace rpc { namespace http {
  
  class path_handler {
    public:
        typedef boost::function<bool(const request&,const std::string&,reply&)> handler_type;

    /// Handle a request and produce a reply.
        void operator()(const request& req, reply& rep);
    
        void register_handler( const std::string& prefix, const handler_type& h );

    private:
       std::map<std::string,handler_type> m_handlers;
  };

}}} // mace::rpc::http
#endif

