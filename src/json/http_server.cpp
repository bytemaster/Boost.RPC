#include <boost/network/protocol/http/server.hpp>
#include <mace/rpc/json/http_server.hpp>

namespace mace { namespace rpc { namespace json { 

  class http_server_private {
    public:
      typedef boost::network::http::server<http_server_private> server;
      
      http_server_private( const std::string& host, uint16_t port, uint16_t t ) 
      :m_server( host, boost::lexical_cast<std::string>(port), *this ) {
         m_thread.reset( new boost::thread( boost::bind(&server::run,&m_server) ) );
      }
      ~http_server_private() {
         m_thread->join();
      }
     
    void operator() (server::request const &req,
                     server::response &response) {
          // TODO: decode request, send response....
        std::cout << "Request Dest: " << req.destination << std::endl;
        std::cout << "Request Method: " << req.method << std::endl;
        std::string bod = body(req);
        std::cout << "bod: '"<<bod<<"'\n";

        server::string_type ip = source(req);
        //std::ostringstream data;
        //data << "Hello, " << ip << "!";
        
        service_map::const_iterator itr = m_services.find( req.destination );
        if( itr != m_services.end() ) {
            json::value call; 
            json::value reply; 
            json::from_string( bod, call );
            itr->second.second->handle_call( call,reply);
            response = server::response::stock_reply(
                server::response::ok, 
                json::to_string(reply)
                );
        } else {

        }

      }

    /*<< It's necessary to define a log function, but it's ignored in
         this example. >>*/
    void log(...) {
     slog("");
        // do nothing
    }

      server m_server;
      boost::shared_ptr<boost::thread> m_thread;
      typedef std::map<std::string, std::pair<boost::any,json::connection::ptr> > service_map;
      service_map m_services;
  };

  http_server::http_server( const std::string& host, uint16_t port, uint16_t t ) {
    my = new http_server_private( host, port, t );
  }
  http_server::~http_server() { delete my; }

  void http_server::add_service( const std::string& path, const rpc::json::connection::ptr& c, const boost::any& s ) {
    my->m_services[path] = std::make_pair(s,c);
  }


} } }
