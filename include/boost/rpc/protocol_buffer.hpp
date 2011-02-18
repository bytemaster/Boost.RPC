#ifndef _BOOST_IDL_PROTOCOL_BUFFER_HPP_
#define _BOOST_IDL_PROTOCOL_BUFFER_HPP_
#include <boost/rpc/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/idl/reflect.hpp>
#include <boost/rpc/detail/protocol_buffer.hpp>
#include <boost/optional.hpp>

namespace boost { namespace rpc { namespace protocol_buffer {
    template<typename T>
    size_t packsize( const T& v )
    {
        boost::rpc::datastream<boost::rpc::packsize>         ps;
        detail::pack_message_visitor<boost::rpc::packsize>   size_visitor( ps );
        visit( const_cast<T&>(v), size_visitor, -1 );
        return ps.tellp();
    }
    template<typename T>
    void pack( char* loc, size_t loc_size, const T& v )
    {
        boost::rpc::datastream<char*>         ds(loc, loc_size);
        detail::pack_message_visitor<char*>   pack_visitor( ds );
        visit( v, pack_visitor, -1 );
    }
    template<typename T>
    void pack( std::vector<char>& msg, const T& v )
    {
        msg.resize(packsize(v));
        pack( &msg.front(), msg.size(), v );
    }
    template<typename T>
    void unpack( const char* msg, size_t msg_size, T& v )
    {
        boost::rpc::datastream<const char*> ds(msg,msg_size);
        detail::unpack_message_visitor( ds, v );
    }

    template<typename T>
    void unpack( const std::vector<char>& msg, T& v )
    {
        if( msg.size() )
            unpack( &msg.front(), msg.size(), v );
    }

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

} } } // namespace boost::rpc

#endif // _BOOST_IDL_PROTOCOL_BUFFER_HPP_
