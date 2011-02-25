#ifndef _BOOST_IDL_PROTOCOL_BUFFER_HPP_
#define _BOOST_IDL_PROTOCOL_BUFFER_HPP_
#include <boost/rpc/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/idl/reflect.hpp>
#include <boost/optional.hpp>

namespace boost { namespace rpc { namespace protocol_buffer {

    template<typename T>
    size_t packsize( const T& v );

    template<typename T>
    void pack( char* loc, size_t loc_size, const T& v );

    template<typename T>
    void pack( std::vector<char>& msg, const T& v );

    template<typename T>
    void pack( std::string& msg, const T& v );

    template<typename T>
    void unpack( const char* msg, size_t msg_size, T& v );

    template<typename T>
    void unpack( const std::vector<char>& msg, T& v );


    /**
     *  Used for RPC template parameter that controls hwo
     *  rpc::client and rpc::server pack/unpack parameters
     *  and results.
     */
    struct protocol
    {
        template<typename T>
        static inline void unpack( const char* msg, size_t msg_size, T& v )
        { protocol_buffer::unpack(msg,msg_size,v); }

        template<typename T>
        static inline void pack( char* msg, size_t msg_size, T& v )
        { protocol_buffer::pack(msg,msg_size,v); }

        template<typename T>
        static inline size_t packsize ( const T& v )
        { return protocol_buffer::packsize(v); }
    };

} } } // namespace boost::rpc

#include <boost/rpc/detail/protocol_buffer.hpp>

#endif // _BOOST_IDL_PROTOCOL_BUFFER_HPP_
