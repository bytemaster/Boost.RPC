#ifndef _BOOST_RPC_JSON_CLIENT_HPP_
#define _BOOST_RPC_JSON_CLIENT_HPP_
#include <boost/reflect/any_ptr.hpp>
#include <boost/rpc/client_interface.hpp>
#include <boost/rpc/json/client_base.hpp>

namespace boost { namespace rpc {  namespace json {

  /**
   *   Given a connection, map methods on InterfaceType into RPC calls
   *   with the expected results returned as futures as defined by rpc::client_interface
   *
   *   @todo - make client_interface take a pointer to client_base to enable the
   *          construction of one client<Interface> and then quickly change the 
   *          connection object without having to iterate over the methods again.
   */
  template<typename InterfaceType>
  class client : public boost::reflect::any_ptr<InterfaceType, boost::rpc::json::client_interface>, 
                 public client_base {
    public:
      typedef boost::shared_ptr<client>   ptr;

      client(){}
      client( const client& c ):client_base(c) {
        boost::rpc::json::client_interface::set( *this );
      }
      bool operator!()const { return !m_con; }

      client& operator=( const client& c ) {
        if( &c != this )  {
            m_con = c.m_con;
            boost::rpc::json::client_interface::set( *this );
        }
        return *this;
      }
      client( const boost::rpc::json::connection::ptr& c) 
      :client_base(c) {
         boost::rpc::json::client_interface::set( *this );
      }      

      // TODO: client( endpoint )
      client( const std::string& host, uint16_t port ) 
      :client_base(host,port){
         boost::rpc::json::client_interface::set( *this );
      }
  };

} } } // boost::rpc::json

#endif
