#ifndef _BOOST_RPC_JSON_CLIENT_HPP_
#define _BOOST_RPC_JSON_CLIENT_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/json.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/rpc/mirror_interface.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/rpc/message.hpp>

namespace boost { namespace rpc { namespace json {
  /**
   * @brief Maps methods on InterfaceType to remote calls via a json::connection
   *
   * The boost::rpc::json::client inherits the interface as 
   * @link boost::reflect::any_ptr boost::reflect::any_ptr<InterfaceType> @endlink
   */
  template<typename InterfaceType>
  class client : public boost::reflect::any_ptr<InterfaceType, boost::rpc::mirror_interface> {
    public:
      typedef boost::shared_ptr<client>                                           ptr;
      typedef boost::reflect::any_ptr<InterfaceType,boost::rpc::mirror_interface> interface_type;

      client( const connection::ptr& c) 
      :m_con(c) {
        c->start();
        boost::reflect::visit( *this, visitor( *this ) ); 
      }      

    private:
      struct visitor {
        visitor( client& s ) :m_self(s){};

        template<typename Member, typename VTable, Member (VTable::*m)>
        inline void operator()( const char* name )const  {
          std::cerr<<"cname: " << name << std::endl;
          detail::if_signal<Member::is_signal>::set_delegate( *m_self.m_con, (*m_self).*m, name ); 
        }
        client& m_self;
      };

      connection::ptr m_con;
  };

} } } // boost::rpc::json

#endif

