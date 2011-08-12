#ifndef _BOOST_RPC_JSON_SERVER_HPP_
#define _BOOST_RPC_JSON_SERVER_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/exception/all.hpp>

namespace boost { namespace rpc { namespace json {

  /**
   * @brief Maps incoming remote calls to any object implementing InterfaceType
   */
  template<typename InterfaceType>
  class server {
    public:
      typedef boost::shared_ptr<server>              ptr;
      typedef boost::reflect::any_ptr<InterfaceType> interface_type;

      template<typename T>
      server( T v, const json::connection::ptr& c )
      :m_interface(v),m_con(c) {
        boost::reflect::visit( m_interface, visitor( *this ) ); 
        m_con->start();
      }

    private:
      struct visitor {
        visitor( server& s ):m_self(s){}

        template<typename Member, typename VTable, Member VTable::*m>
        void operator()(const char* name )const  {
             std::cerr<<"sname:"<<name<<std::endl;
             m_self.m_con->add_method_handler( name, 
                detail::rpc_recv_functor<typename Member::fused_params, 
                                 Member&, Member::is_signal>((*m_self.m_interface).*m,*m_self.m_con,name) );
        }
        server&         m_self;
      };

      connection::ptr m_con;
      interface_type  m_interface; 
  };

} } } // boost::rpc::json

#endif
