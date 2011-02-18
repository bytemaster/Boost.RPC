#ifndef _BOOST_IDL_PROTOCOL_BUFFER_HPP_
#define _BOOST_IDL_PROTOCOL_BUFFER_HPP_
#include <boost/rpc/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/idl/reflect.hpp>
#include <boost/optional.hpp>

namespace boost { namespace rpc { namespace protocol_buffer {

    template<typename T>
    struct required : public boost::optional<T>
    {
        typedef boost::optional<T> base;
        required(){}
        required( const T& v )
        :boost::optional<T>(v){}

        using base::operator=;
        using base::operator*;
        using base::operator!;
    };

    template<typename T>
    size_t packsize( const T& v );

    template<typename T>
    void pack( char* loc, size_t loc_size, const T& v );

    template<typename T>
    void pack( std::vector<char>& msg, const T& v );

    template<typename T>
    void unpack( const char* msg, size_t msg_size, T& v );

    template<typename T>
    void unpack( const std::vector<char>& msg, T& v );

} } } // namespace boost::rpc

#include <boost/rpc/detail/protocol_buffer.hpp>

#endif // _BOOST_IDL_PROTOCOL_BUFFER_HPP_
