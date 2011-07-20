#ifndef _BOOST_RPC_JSON_CLIENT_HPP_
#define _BOOST_RPC_JSON_CLIENT_HPP_
#include <boost/rpc/json.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/rpc/message.hpp>

namespace boost { namespace rpc { namespace json {
     namespace detail {

        class client_base : public boost::enable_shared_from_this<client_base> {
            public:
                client_base( const boost::rpc::json::connection::ptr& c );
                ~client_base();

                void invoke( js::Value& msg, js::Value& rtn_msg, uint64_t timeout_us = -1 );
            private:
                class client_base_private* my;
        };

        template<typename Seq,typename ResultType=void>
        struct rpc_functor {
            rpc_functor( client_base& c, const char* name )
            :m_client(c),m_msg(js::Object())
            {
                 js::Object&  m_obj = m_msg.get_obj(); // obj stored in m_msg
                 m_obj.push_back( js::Pair( "id", 0 ) );
                 m_obj.push_back( js::Pair( "method", std::string(name) ) );
                 m_obj.push_back( js::Pair( "params", js::Array() ) );
            }

            ResultType operator()( const Seq& params ) {
                 js::Object&  m_obj = m_msg.get_obj(); // obj stored in m_msg
                 pack( m_obj.back().value_, params );
                 js::Value  rtn_msg;
                 m_client.invoke( m_msg, rtn_msg );
                 ResultType  ret_val;
                 if( rtn_msg.contains( "result" ) )  {
                     unpack( rtn_msg["result"], ret_val );
                     return ret_val;
                 }
                 if(rtn_msg.contains("error") ) {
                    error_object e;
                    unpack( rtn_msg["error"], e );
                    BOOST_THROW_EXCEPTION( e );
                 }
	         error_object e;
		 e.message = "invalid json RPC message, missing result or error";
	         BOOST_THROW_EXCEPTION( e );
            }
            client_base& m_client;
            js::Value    m_msg;
        }; // rpc_functor

    }  // namespace detail 

    template<typename InterfaceType>
    class client: public boost::reflect::visitor< client<InterfaceType> >, 
                  public boost::reflect::any<InterfaceType>,
                  public detail::client_base
    {
       public:
           typedef boost::shared_ptr<client> ptr;
           client( const boost::rpc::json::connection::ptr& c)
           :client_base(c) {
               start_visit(*this); 
           }
           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name ) {
                return m.m_delegate = detail::rpc_functor<typename M::fused_params, typename M::result_type>(*this,name);
           }
    };

} } } // boost::rpc::json

#endif

