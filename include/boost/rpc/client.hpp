#ifndef _BOOST_RPC_CLIENT_HPP
#define _BOOST_RPC_CLIENT_HPP
#include <boost/reflect/any.hpp>
#include <boost/rpc/detail/client_base.hpp>
#include <boost/rpc/serialization.hpp>
#include <sstream>
#include <boost/system/system_error.hpp>

namespace boost { namespace rpc {

    template<typename InterfaceType>
    class client: public boost::reflect::visitor< client<InterfaceType> >, 
                  public boost::reflect::any<InterfaceType>,
                  public detail::client_base
    {
       public:
           typedef boost::shared_ptr<client> ptr;
           client() {
               start_visit(*this);
           }

           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name ) {
                m.m_delegate = rpc_functor<typename M::fused_params, typename M::result_type>(*this,name);
                return true;
           }

       private:
           template<typename Seq, typename ResultType>
           friend struct rpc_functor;

           template<typename Seq, typename ResultType>
           struct rpc_functor
           {
               rpc_functor( client& c, const char* name )
               :m_client(c),m_name(name){}

               ResultType operator()( const Seq& params ) {
                    std::ostringstream os;
                    {  
                        boost::archive::binary_oarchive oa(os);
                        serialization::serialize_fusion_vector(oa, params);
                    }
                    ResultType  ret_val;
                    std::istringstream is( m_client.invoke( m_name, os.str() ) );
                    {
                        boost::archive::binary_iarchive ia(is);
                        ia >> ret_val;
                    }
                    return ret_val;
               }
               const char* m_name;
               client& m_client;
           };
    };


} } // namespace boost::rpc


/**
 *  This is provided to enable rpc::client<T> to be assigned to reflect::any<T>, in which case
 *  every method of the any gets assigned to the value of the delegate of the client.
 */
namespace boost { namespace reflect {
    template<typename T>
    class set_delegate_visitor<boost::rpc::client<T> > : 
        public boost::reflect::visitor<set_delegate_visitor<boost::rpc::client<T> > >
    {
        public:
           typedef boost::rpc::client<T>* type;

           set_delegate_visitor( boost::rpc::client<T>* self = 0)
           :m_self(self){}

           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name ) {
                M::get_member_on_type(m_self,m.m_delegate);
                return true;
           }
       private:
           boost::rpc::client<T>* m_self;
    };
} }

#endif // _BOOST_RPC_CLIENT_HPP

