#ifndef _BOOST_RPC_CONNECTION_HPP
#define _BOOST_RPC_CONNECTION_HPP
#include <boost/function.hpp>
#include <boost/cmt/retainable.hpp>
#include <boost/bind.hpp>
#include <boost/rpc/message.hpp>

namespace boost { namespace rpc {

/**
 *  @class boost::rpc::connection
 *  @brief Abstracts a connection between a client & server.
 *
 *  The underlying medium (TCP/UDP/UDT/SSL/etc) should be hidden
 *  behind this interface.
 *
 *  Anytime a message is received, the received_message delegate should
 *  be called.  If the connection is close, then the closed delegate
 *  should be called.
 *
 *  No assumptions are made regarding the thread the callback occurs
 *  from. 
 *
 *  The Protocol template parameter controls how the message, parameters,
 *  errors, and return values are packed. 
 */
struct connection : public boost::cmt::retainable
{
    typedef boost::cmt::retainable_ptr<connection<Protocol> >  ptr;
    virtual void  send_message( const message& m ) = 0;

    protected:
        void received_message( const message& m ) { if( _received_message ) _received_message(m); }
        void connection_closed()                  { if( _closed ) _closed();                      }

    private:
        template<typename>
        friend class server;
        template<typename>
        friend class client;
        boost::function<void(const message& )> _received_message;
        boost::function<void()>                _closed;
};

} }

#endif // _BOOST_RPC_CONNECTION_HPP
