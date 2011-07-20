#include <boost/rpc/json/server.hpp>
#include <boost/rpc/message.hpp>

namespace boost { namespace rpc { namespace json {
     namespace detail {
         class server_base_private {
            public:
                std::vector<boost::rpc::json::connection::ptr> connections;
                server_base*                                   self;

                void on_recv( const boost::rpc::json::connection::ptr& con, const js::Value& v ) {
                    std::string       name  = v["method"].get_str();
                    js::Value result = js::Object();
                    result["id"]     = v["id"];

                    try {
                        server_base::method_map::const_iterator itr = self->m_methods.find(name);
                        if( itr != self->m_methods.end() ) {
                            itr->second( v["params"], result["result"] );
                        } else {
                            boost::rpc::error_object eobj(0);
                            eobj.message = "Unknown method '"+name+"'";
                            pack( result["error"], eobj );
                        }
                    } catch ( const boost::exception& e ) {
                        boost::rpc::error_object eobj(0);
                        eobj.message = boost::diagnostic_information(e);
                        pack( result["error"], eobj );
                    }
                    con->send(result);
                }
         };

         server_base::server_base() {
            my = new server_base_private();
            my->self = this;
         }
         server_base::~server_base() {
            delete my;
         }
         void server_base::add_connection( const boost::rpc::json::connection::ptr& con ) {
            my->connections.push_back(con);
            con->set_recv_handler( boost::bind( &server_base_private::on_recv, my, con, _1 ) );
         }
     }

} } } 
