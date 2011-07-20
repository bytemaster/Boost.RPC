#ifndef _BOOST_RPC_VARINT_HPP_
#define _BOOST_RPC_VARINT_HPP_
#include <boost/reflect/reflect.hpp>
#include <boost/rpc/datastream.hpp>

namespace boost { namespace rpc {

struct unsigned_int
{
    unsigned_int( uint32_t v = 0 ):value(v){}

    operator uint32_t()const { return value; }

    template<typename T>
    unsigned_int& operator=( const T& v ) { value = v; return *this; }
    
    uint32_t value;

    template<typename T>
    friend inline datastream<T>& operator<<( datastream<T>& ds, const unsigned_int& vi )
    {
        uint64_t v = vi.value;
        do 
        {
           uint8_t b = uint8_t(v) & 0x7f;
           v >>= 7;
           b |= ((v > 0) << 7);
           ds.putc(b);
        }while( v );
        return ds;
    }

    template<typename T>
    friend inline datastream<T>& operator>>( datastream<T>& ds, unsigned_int& vi )
    {
        uint64_t v = 0;
        char b = 0;
        uint8_t by = 0;
        do {
            ds.getc(b);
            v |= uint32_t(uint8_t(b) & 0x7f) << by;
            by += 7;
        } while( uint8_t(b) & 0x80 );

        vi.value = v;

        return ds;
    }
};

struct signed_int
{
    signed_int( int32_t v = 0 ):value(v){}
    operator int32_t()const { return value; }
    template<typename T>
    signed_int& operator=( const T& v ) { value = v; return *this; }

    int32_t value;

    template<typename T>
    friend inline datastream<T>& operator<<( datastream<T>& ds, const signed_int& vi )
    {
        uint32_t v = (vi.value<<1) ^ (vi.value>>31);
        do
        {
           uint8_t b = uint8_t(v) & 0x7f;
           v >>= 7;
           b |= ((v > 0) << 7);
           ds.putc(b);
        }
        while( v );
        return ds;
    }

    template<typename T>
    friend inline datastream<T>& operator>>( datastream<T>& ds, signed_int& vi )
    {
        uint32_t v = 0;
        char b = 0;
        int by = 0;
        do {
            ds.getc(b);
            v |= uint32_t(uint8_t(b) & 0x7f) << by;
            by += 7;
        } while( uint8_t(b) & 0x80 );

        vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
        vi.value = v&0x01 ? vi.value : -vi.value;
        vi.value =  - vi.value;

        return ds;
    }
};

} }

BOOST_REFLECT_TYPEINFO( boost::rpc::unsigned_int )
BOOST_REFLECT_TYPEINFO( boost::rpc::signed_int )

#endif 
