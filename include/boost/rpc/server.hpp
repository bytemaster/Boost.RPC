#ifndef _BOOST_RPC_SERVER_HPP_
#define _BOOST_RPC_SERVER_HPP_
#include <boost/rpc/debug.hpp>
#include <boost/rpc/connection.hpp>
#include <boost/rpc/message.hpp>
#include <boost/rpc/json.hpp>
#include <boost/function.hpp>
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
        void add_connection( const boost::shared_ptr<connection<Protocol> >& c );

    private:
        void handle_message( const typename boost::rpc::connection<Protocol>::ptr& con, const message& m )
        {
//           dlog( "handle message: ", to_json(m) );
           if( m.method )
           {
                method_map::iterator itr = std::find( m_methods.begin(), m_methods.end(), *m.method );
                if( itr != m_methods.end() )
                {
                    message reply;
                    reply.id = m.id;
                    try {
                        reply.result = itr->second(m.params ? *m.params : std::string() );
                    } catch ( boost::exception& e ) {
                        reply.error = error_object( -1, boost::diagnostic_information(e) );
                    }
                    con->send_messag(reply);
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
                    con->send_messag(reply);
                }
           }
        }
        
        typedef std::pair<std::string,boost::function<std::string(const std::string&)> > name_func;
        typedef std::vector<name_func> method_map;
        
        method_map                           m_methods;
        std::list<typename connection<Protocol>::ptr> m_connections;
};


} } // namespace boost::rpc

#endif
