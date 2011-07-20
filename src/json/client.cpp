#include <boost/rpc/json/client.hpp>
#include <boost/cmt/thread.hpp>

namespace boost { namespace rpc { namespace json {
     namespace detail {
        class client_base_private { 
            public:
                typedef std::map<int, boost::cmt::promise<js::Value>* > pending_req_map;
                pending_req_map                   pending_req;
                int                               next_id;
                boost::rpc::json::connection::ptr con;

                void on_recv( const js::Value& v ) {
                    if( v.contains( "id" ) ) {
                        int id = v["id"].get_int();
                        pending_req_map::iterator itr = pending_req.find(id);
                        if( itr != pending_req.end() ) {
                            itr->second->set_value(v);
                            pending_req.erase(itr);
                        } else {
                            elog( "unhandled message: '%1%'", json_spirit::write(v) );
                        }
                    } else {
                        elog( "message has no 'id' field: '%1%'", json_spirit::write(v) );
                    }
                }
        };

        client_base::client_base( const boost::rpc::json::connection::ptr& c ) 
        :my(new client_base_private() ) {
            my->next_id = 0;
            my->con     = c;
            my->con->set_recv_handler( boost::bind(&client_base_private::on_recv,my,_1) );
        }

        client_base::~client_base() {
            delete my;
        }

        void  client_base::invoke( js::Value& msg, js::Value& rtn_msg, uint64_t timeout_us  ) {
            int id = ++my->next_id;
            msg.get_obj()[0].value_ = id;

            boost::cmt::stack_retainable<boost::cmt::promise<js::Value> > p;
            my->pending_req[id] = &p;

            my->con->send( msg );
            try {
                rtn_msg = p.wait(timeout_us);
            } catch ( const boost::exception& e ) {
                my->pending_req.erase(my->pending_req.find(id));
                throw;
            }
        }
    
     } // detail
}}} // boost::rpc::json
