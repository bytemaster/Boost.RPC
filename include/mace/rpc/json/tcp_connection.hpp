#ifndef _MACE_RPC_JSON_TCP_CONNECTION_HPP_
#define _MACE_RPC_JSON_TCP_CONNECTION_HPP_
#include <mace/rpc/json/connection.hpp>
#include <mace/cmt/asio/tcp/socket.hpp>
#include <boost/asio.hpp>

namespace mace { namespace rpc { namespace json {

class tcp_connection : public mace::rpc::json::connection {
  public:
      typedef boost::shared_ptr<tcp_connection> ptr;
      typedef boost::weak_ptr<tcp_connection>   wptr;

      /**
       *  @param t - the thread in which messages will be sent and callbacks invoked
       */
      tcp_connection( const mace::cmt::asio::tcp::socket::ptr& sock,
                      mace::cmt::thread* t = &mace::cmt::thread::current() );


      tcp_connection( const std::string& host, uint16_t port, mace::cmt::thread* t = &mace::cmt::thread::current() );
      virtual ~tcp_connection();

  protected:
      friend class tcp_connection_private;
      virtual void send( const rpc::json::value& msg );

      class tcp_connection_private* my;
};

} } } // namespace mace::rpc::json

#endif //_MACE_RPC_TCP_CONNECTION_HPP_
