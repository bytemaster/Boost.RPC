#include <boost/rpc/json/tcp/connection.hpp>
#include <boost/asio.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {
    typedef boost::cmt::asio::tcp::socket socket_t;
    typedef boost::function<void(const connection::ptr&)> handler;

    namespace detail {
        void listen( uint16_t port, const handler& handle ) {
            try {
              boost::asio::ip::tcp::acceptor acc( boost::cmt::asio::default_io_service(), 
                                                  boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),port) );
             
              boost::system::error_code ec;
              do {
                  socket_t::ptr iosp(new socket_t());
                  ec = boost::cmt::asio::tcp::accept( acc, *iosp);
                  if(!ec) {
                      boost::cmt::async( boost::bind(handle, connection::ptr( new connection(iosp) ) )); 
                  }
              }while( !ec );
              boost::cmt::async(boost::bind(handle,connection::ptr()));
            } catch ( const boost::exception& e  ) {
                elog( "%1%", boost::diagnostic_information(e) );
            }
        }
    }

    void listen( uint16_t port, const handler& handle ) {
        boost::cmt::async( boost::bind( &boost::rpc::json::tcp::detail::listen, port, handle ) );
    }

} } } } // boost::rpc::json::tcp
