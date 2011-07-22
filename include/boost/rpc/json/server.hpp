#ifndef _BOOST_RPC_JSON_SERVER_HPP_
#define _BOOST_RPC_JSON_SERVER_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/exception/all.hpp>

namespace boost { namespace rpc { namespace json {

    template<typename InterfaceType>
    class server {
        public:
            template<typename T>
            server( T v, const connection::ptr& c )
            :m_interface(v),m_con(c) {
                visitor vi(*c);
                boost::reflect::reflector<InterfaceType>::visit( *m_interface, vi );
                m_con->start();
            }

        private:
            struct visitor {
                visitor( connection& c ):m_con(c){};
                template<typename InterfaceName, typename M>
                bool accept( M& m, const char* name ) {
                     slog( "add method %1%", name );
                     m_con.add_method_handler( name, 
                        detail::rpc_recv_functor<typename M::fused_params, 
                                         M&, M::is_signal>(m,m_con,name) );
                }
                connection& m_con;
            };
            connection::ptr                        m_con;
            boost::reflect::any_ptr<InterfaceType> m_interface; 
    };

} } } // boost::rpc::json

#endif
