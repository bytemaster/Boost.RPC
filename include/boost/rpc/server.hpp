#ifndef _BOOST_RPC_SERVER_HPP_
#define _BOOST_RPC_SERVER_HPP_
#include <boost/rpc/debug.hpp>
#include <boost/rpc/connection.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/rpc/message.hpp>
#include <boost/utility.hpp>
#include <boost/rpc/json.hpp>
#include <boost/function.hpp>
#include <boost/typeof/typeof.hpp>
#include <string>
#include <map>
#include <vector>

namespace boost { namespace rpc {

/**
 * @class boost::rpc::server
 *
 * @brief Maps RPC requests to functors and returns the result.
 *
 */
template<typename Protocol>
class server
{
    public:
        void add_connection( const boost::shared_ptr<connection<Protocol> >& c )
        {
            m_connections.push_back(c);
            c->_received_message =  boost::bind( &server::handle_message, this, c, _1 );
       //     c->_closed.connect( boost::bind( &server::handle_message, this, c, _1 ) );
        }

        template<typename Functor>
        void add_method( const std::string& name, Functor f )
        {
            m_methods[name] = rpc_functor<Functor>(f); 
        }

    private:
        template<typename Functor>
        struct rpc_functor
        {
            rpc_functor( Functor f )
            :m_func(f){}

            std::string operator()( const std::string& params )
            {
                 BOOST_TYPEOF(m_func(params)) r = m_func( params );
                 size_t s = Protocol::packsize(r);
                 std::string msg;
                 msg.resize(s);
                 Protocol::pack((char*)msg.c_str(), s, r );
                 return msg;
            }
            Functor m_func;
        };
        


        void handle_message( const typename boost::rpc::connection<Protocol>::ptr& con, const message& m )
        {
           elog( "handle message: %1%", to_json(m) );
           std::cerr<< to_json(m) << std::endl;
           if( m.method )
           {
                method_map::iterator itr = m_methods.find(*m.method); 
                if( itr != m_methods.end() )
                {
                    message reply;
                    reply.id = m.id;
                    try {
                        reply.result = itr->second(m.params ? *m.params : std::string() );
                    } catch ( boost::exception& e ) {
                        reply.error = error_object( -1, boost::diagnostic_information(e) );
                    }
                    con->send_message(reply);
                }
                else
                {
                    boost::rpc::error_object err;
                    err.code = -1;
                    err.message = "Unknown method name '" + *m.method + "'";
                    elog( "Unknown method name %1%", *m.method );

                    message reply;
                    reply.id = m.id;
                    reply.error = err;
                    con->send_message(reply);
                }
           }
           else
           {
                boost::rpc::error_object err;
                err.code = -1;
                err.message = "method name not specified";
                elog( "message %1%", *err.message );

                message reply;
                reply.id = m.id;
                reply.error = err;
                con->send_message(reply);
           }
        }
        
        typedef std::pair<std::string,boost::function<std::string(const std::string&)> > name_func;
        typedef std::map<std::string,boost::function<std::string(const std::string&)> > method_map;
        //name_func> method_map;
        
        method_map                           m_methods;
        std::list<typename connection<Protocol>::ptr> m_connections;
};


} } // namespace boost::rpc

#endif
