#ifndef _MACE_RPC_JSON_TCP_SERVER_HPP_
#define _MACE_RPC_JSON_TCP_SERVER_HPP_
#include <mace/cmt/thread.hpp>
#include <mace/stub/ptr.hpp>
#include <mace/rpc/json/value_io.hpp>
#include <mace/rpc/json/detail/tcp_server_base.hpp>

namespace mace { namespace rpc { namespace json {
 
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
                  mace::cmt::thread* t = &mace::cmt::thread::current() )
      :tcp_server_base( new session_creator_impl<SessionType>(sg), port, t ){}

      template<typename SessionType>
      tcp_server( const boost::shared_ptr<SessionType>& shared_session, uint16_t port,
                 mace::cmt::thread* t = &mace::cmt::thread::current() ) 
      :tcp_server_base( new shared_session_creator<SessionType>(shared_session), port, t ){}

    private:
      template<typename SessionType>
      struct session_creator_impl : public session_creator {
          session_creator_impl( const boost::function<boost::shared_ptr<SessionType>()>& sg )
          :session_generator(sg){ }

          virtual boost::any init_connection( const rpc::json::connection::ptr& con ) {
            mace::stub::ptr<InterfaceType> session( session_generator() );
            mace::stub::visit( session, add_interface_visitor<InterfaceType>( *con, session ) );
            return session;
          }
          boost::function<boost::shared_ptr<SessionType>()> session_generator;
      };

      template<typename SessionType>
      struct shared_session_creator : public session_creator {
          shared_session_creator( const boost::shared_ptr<SessionType>& ss ):shared_session(ss){}

          virtual boost::any init_connection( const rpc::json::connection::ptr& con ) {
            mace::stub::ptr<InterfaceType> session;
            session = shared_session;
            mace::stub::visit( session, add_interface_visitor<InterfaceType>( *con, session ) );
            return session;
          }
          boost::shared_ptr<SessionType> shared_session;
      };

  };

} } } // mace::rpc::json

#endif // _MACE_RPC_JSON_TCP_SERVER_HPP_
