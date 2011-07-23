#ifndef _BOOST_RPC_JSON_CLIENT_HPP_
#define _BOOST_RPC_JSON_CLIENT_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/json.hpp>
#include <boost/rpc/json/connection.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/rpc/message.hpp>

namespace boost { namespace rpc { namespace json {
  /**
   * @brief Maps methods on InterfaceType to remote calls via a json::connection
   */
  template<typename InterfaceType>
  class client : public boost::reflect::any_ptr<InterfaceType>
  {
    public:
      typedef boost::shared_ptr<client>              ptr;
      typedef boost::reflect::any_ptr<InterfaceType> interface_type;

      client( const connection::ptr& c) 
      :m_con(c) {
        c->start();
        visitor vi(*c);
        boost::reflect::reflector<InterfaceType>::visit( **this, vi );
      }      
    private:
      struct visitor {
        visitor( connection& c ):m_con(c){};

        template<typename InterfaceName, typename M>
        bool accept( M& m, const char* name ) {
          detail::if_signal<M::is_signal>::set_delegate( m_con, m, name ); 
          return true;
        }
        connection& m_con;
      };
      connection::ptr m_con;
  };

} } } // boost::rpc::json

#endif

