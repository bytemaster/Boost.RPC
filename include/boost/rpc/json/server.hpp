#ifndef _BOOST_RPC_JSON_SERVER_HPP_
#define _BOOST_RPC_JSON_SERVER_HPP_
#include <boost/rpc/json/connection.hpp>
#include <boost/exception/all.hpp>

namespace boost { namespace rpc { namespace json {

    namespace detail {
        class server_base {
            public:
                server_base();
                virtual ~server_base();

                void add_connection( const boost::rpc::json::connection::ptr& con );
                
            protected:
                typedef std::map<std::string, boost::function<void(const js::Value&, js::Value&)> > method_map;
                method_map   m_methods;

                friend class server_base_private;
                class server_base_private* my;
        };
        

       template<typename Seq, typename Functor>
       struct rpc_functor {
           rpc_functor( Functor f ):m_func(f){}
           void operator()( const js::Value& params, js::Value& rtn ) {
               Seq paramv;
               unpack( params, paramv );
               //    typename boost::remove_reference<Functor>::type::result_type r = 
               pack( rtn, m_func(paramv) );
           }
           Functor m_func;
       };

    } // namespace detail

    template<typename InterfaceType>
    class server : public boost::reflect::any<InterfaceType>, 
                   public boost::reflect::visitor<server<InterfaceType> >, 
                   public detail::server_base {
        public:
            template<typename T>
            server( T v )
            :reflect::any<InterfaceType>(v) {
                 start_visit(*this); 
            }

            template<typename InterfaceName, typename M>
            bool accept( M& m, const char* name ) {
                 m_methods[name] = detail::rpc_functor<typename M::fused_params, M&>(m);
                 return true;
            }
    };

} } } // boost::rpc::json

#endif
