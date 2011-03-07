#ifndef _BOOST_RPC_TCP_CONNECTION_HPP_
#define _BOOST_RPC_TCP_CONNECTION_HPP_
#include <boost/rpc/connection.hpp>
#include <boost/rpc/debug.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace boost { namespace rpc { namespace tcp {

    /**
     *  @class boost::rpc::tcp::connection
     *  @brief Sends/Receives messages over a boost::asio::tcp::socket
     *
     *  Messages are packetized as 
     *
     *      { 
     *        int32_t size  
     *        data[size] 
     *      }
     *
     *  size = Protocol::packsize(message)
     *  Protocol::pack( data, size, message )
     *
     */
    template<typename Protocol>
    class connection : public boost::rpc::connection<Protocol>
    {
        public:
            connection( const boost::shared_ptr<boost::asio::ip::tcp::socket>& sock )
            :m_sock(sock)
            {
                start_read_message();
            }
                            
            void send_message( const message& m )
            {
                uint32_t ps = Protocol::packsize(m);
                elog( "packsize %1%", ps );
                if( ps ) {
                    char* data = new char[ps+sizeof(ps)];
                    memcpy( data, &ps, sizeof(ps) );
                    Protocol::pack( data+4, ps, m );

                    boost::asio::async_write( *m_sock, boost::asio::buffer( data, ps+sizeof(ps) ),
                                              boost::bind( &connection::write_completion_handler, this, data, _1, _2 ) );
                }
            }

        private:
            void start_read_message()
            {
                boost::asio::async_read( *m_sock, boost::asio::buffer( (char*)&m_msg_size, sizeof(m_msg_size) ),
                                         boost::bind( &connection::handle_read_msg_size, 
                                         this, _1, _2 ) );
            }

            void write_completion_handler( char* data,
                                           const boost::system::error_code& err,
                                           std::size_t bytes_transferred)
            {
                delete[] data;
            }

            void handle_read_msg_body( const boost::system::error_code& err, std::size_t bytes_transfered )
            {
                dlog( "size %1% m_rbuf.size() %2%", bytes_transfered, m_rbuf.size() );
                if( err ) elog( "error! %1%", err.message() );
                if( !err && bytes_transfered ) {
                    message msg;
                    Protocol::unpack( &m_rbuf.front(), bytes_transfered, msg );
                    start_read_message();
                    this->received_message(msg);
                } else {
                    elog( "error" );
                    this->connection_closed();
                }
            }
            void handle_read_msg_size(const boost::system::error_code& err, std::size_t bytes_transfered )
            {
                dlog( "transfered %1%  m_msg_size %2% ", bytes_transfered, m_msg_size );
                if( !err ) {
                    if( m_msg_size > 1024 * 1024 ) {
                        assert( !"Message size too big!" );
                        this->connection_closed();
                        return;
                    }
                    if( m_msg_size > 0 ) {
                        m_rbuf.resize(m_msg_size);
                        boost::asio::async_read( *m_sock, boost::asio::buffer( m_rbuf ),
                                                 boost::bind( &connection::handle_read_msg_body, 
                                                 this, _1, _2 ) );
                    } else {
                        start_read_message();
                    }
                } else {
                    elog( "error! %1%", err.message() );
                    this->connection_closed();
                }
            }
            uint32_t                                        m_msg_size;
            std::vector<char>                               m_rbuf;
            boost::shared_ptr<boost::asio::ip::tcp::socket> m_sock;
    };

} } } // boost::rpc::tcp

#endif
