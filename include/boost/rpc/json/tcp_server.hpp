#ifndef _BOOST_RPC_JSON_TCP_SERVER_HPP_
#define _BOOST_RPC_JSON_TCP_SERVER_HPP_
#include <boost/cmt/thread.hpp>
#include <boost/reflect/any_ptr.hpp>
#include <boost/fusion/support/deduce_sequence.hpp>
#include <boost/rpc/json/value_io.hpp>
#include <boost/rpc/json/detail/tcp_server_base.hpp>

namespace boost { namespace rpc { namespace json {
 
  /**
   *  A services listens for new incoming tcp connections and opens
   *  a JSON-RPC tcp_connection with InterfaceType's method.
   *
   *  Each connection has the opportunity to have a unique session by
   *  creating a new instance of SessionType and providing a functor
   *  that will generate a new session.  
   *
   *  The simplest case is the shared session in which all connections
   *  invoke methods on the same instance of InterfaceType.
   */
  template<typename InterfaceType>
  class tcp_server : public detail::tcp_server_base {
    public:
      typedef boost::shared_ptr<tcp_server> ptr;

      template<typename SessionType>
      tcp_server( const boost::function<boost::shared_ptr<SessionType>()>& sg, uint16_t port, 
                  boost::cmt::thread* t = &boost::cmt::thread::current() )
      :tcp_server_base( new session_creator_impl<SessionType>(sg), port, t ){}

      template<typename SessionType>
      tcp_server( const boost::shared_ptr<SessionType>& shared_session, uint16_t port,
                 boost::cmt::thread* t = &boost::cmt::thread::current() ) 
      :tcp_server_base( new shared_session_creator<SessionType>(shared_session), port, t ){}

    private:
      template<typename SessionType>
      struct session_creator_impl : public session_creator {
          session_creator_impl( const boost::function<boost::shared_ptr<SessionType>()>& sg )
          :session_generator(sg){ }

          virtual boost::any init_connection( const rpc::json::connection::ptr& con ) {
            boost::reflect::any_ptr<InterfaceType> session( session_generator() );
            boost::reflect::visit( session, visitor( *con, session ) );
            return session;
          }
          boost::function<boost::shared_ptr<SessionType>()> session_generator;
      };

      template<typename SessionType>
      struct shared_session_creator : public session_creator {
          shared_session_creator( const boost::shared_ptr<SessionType>& ss ):shared_session(ss){}

          virtual boost::any init_connection( const rpc::json::connection::ptr& con ) {
            boost::reflect::any_ptr<InterfaceType> session;
            session = shared_session;
            boost::reflect::visit( session, visitor( *con, session ) );
            return session;
          }
          boost::shared_ptr<SessionType> shared_session;
      };

      /**
       *  Visits each method on the any_ptr<InterfaceType> and adds it to the connection object.
       */
      struct visitor {
        visitor( rpc::json::connection& c, boost::reflect::any_ptr<InterfaceType>& s )
        :m_con(c),m_aptr(s){}
      
        template<typename Member, typename VTable, Member VTable::*m>
        void operator()(const char* name )const  {
             typedef typename boost::fusion::traits::deduce_sequence<typename Member::fused_params>::type param_type;
             m_con.add_method( name, detail::rpc_recv_functor<param_type, Member&>( (*m_aptr).*m ) );
        }
        rpc::json::connection&                   m_con;
        boost::reflect::any_ptr<InterfaceType>& m_aptr;
      };
  };

} } } // boost::rpc::json

#endif // _BOOST_RPC_JSON_TCP_SERVER_HPP_
