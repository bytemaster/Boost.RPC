/*
 *  sha1.hpp
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved.
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in this class, especially the single
 *      character names, were used because those were the names used
 *      in the publication.
 *
 *      Please read the file sha1.cpp for more information.
 *
 */

#ifndef _BOOST_RPC_DATASTREAM_SHA1_HPP_
#define _BOOST_RPC_DATASTREAM_SHA1_HPP_
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/rpc/datastream/datastream.hpp>
#include <boost/reflect/reflect.hpp>

namespace boost { namespace rpc {

struct sha1_hashcode
{
    uint32_t hash[5];

    sha1_hashcode( )
    {
        memset(hash, 0, sizeof(hash) );
    }
    sha1_hashcode( const sha1_hashcode& c )
    {
        memcpy(hash, c.hash, sizeof(hash) );    
    }
    sha1_hashcode& operator = ( const sha1_hashcode& b )
    {
        memcpy( hash, b.hash, sizeof(hash) );
        return *this;
    }
    std::string str()const;
};


inline sha1_hashcode operator << ( const sha1_hashcode& h1, uint32_t i )
{
    sha1_hashcode result;
    uint8_t* r = (uint8_t*)result.hash;
    uint8_t* s = (uint8_t*)h1.hash;
    for( uint32_t p = 0; p < 19; ++p )
        r[p] = s[p] << i | (s[p+1]>>(8-i));
    r[19] = s[19] << i;
    return result;
}
inline sha1_hashcode operator ^ ( const sha1_hashcode& h1, const sha1_hashcode h2 )
{
    sha1_hashcode result;
    result.hash[0] = h1.hash[0] ^ h2.hash[0];
    result.hash[1] = h1.hash[1] ^ h2.hash[1];
    result.hash[2] = h1.hash[2] ^ h2.hash[2];
    result.hash[3] = h1.hash[3] ^ h2.hash[3];
    result.hash[4] = h1.hash[4] ^ h2.hash[4];
    return result;
}
inline bool operator >= ( const sha1_hashcode& h1, const sha1_hashcode h2 )
{
    return memcmp( h1.hash, h2.hash, sizeof(h1.hash) ) >= 0;
}
inline bool operator > ( const sha1_hashcode& h1, const sha1_hashcode h2 )
{
    return memcmp( h1.hash, h2.hash, sizeof(h1.hash) ) > 0;
}

template<typename T>
datastream<T>& operator<<( datastream<T>& ds, const sha1_hashcode& ep )
{
    ds.write( (const char*)ep.hash, sizeof(ep.hash) );
    return ds;
}
template<typename T>
datastream<T>& operator>>( datastream<T>& ds, sha1_hashcode& ep )
{
    ds.read( (char*)ep.hash, sizeof(ep.hash) );
    return ds;
}


inline bool operator == ( const sha1_hashcode& a, const sha1_hashcode& b )
{
   return memcmp( a.hash, b.hash, sizeof(a.hash) ) == 0; 
}
inline bool operator != ( const sha1_hashcode& a, const sha1_hashcode& b )
{
   return memcmp( a.hash, b.hash, sizeof(a.hash) ) != 0; 
}
inline bool operator < ( const sha1_hashcode& a, const sha1_hashcode& b )
{
   return memcmp( a.hash, b.hash, sizeof(a.hash) ) < 0; 
}
inline std::ostream& operator<< ( std::ostream& os, const sha1_hashcode& h )
{
    uint8_t* c = (uint8_t*)h.hash;
    for( uint32_t i = 0; i < 20; ++i )
        os << std::hex << (c[i]>>4) << std::hex << (c[i] &0x0f);
    return os;
}
inline std::string sha1_hashcode::str()const
{
    std::stringstream ss; 
    ss << *this;
    return ss.str();
}
inline std::istream& operator>>(std::istream& is, sha1_hashcode& h )
{
    is >> std::hex >> h.hash[0] >> h.hash[1] >> h.hash[2] >> h.hash[3] >> h.hash[4];
    return is;
} 

namespace detail {

class sha1_encoder
{
    public:
        sha1_encoder() { reset(); }
        ~sha1_encoder(){};

        /*
         *  Re-initialize the class
         */
        void reset();

        /*
         *  Returns the message digest
         */
        bool result(unsigned *message_digest_array);
        sha1_hashcode result()
        {
            sha1_hashcode h;
            result( (unsigned*)h.hash );
            return h;
        }

        /*
         *  Provide input to sha1
         */
        void input( const unsigned char *message_array,
                    unsigned            length);
        void input( const char  *message_array,
                    unsigned    length);
        void input(unsigned char message_element);
        void input(char message_element);

        static sha1_hashcode hash( const std::string& str )
        {
            sha1_encoder sh;
            sh.input( str.c_str(), str.size() );
            sha1_hashcode hc;
            sh.result( (unsigned*)hc.hash );
            return hc;
        }
        static sha1_hashcode hash( const std::vector<char>& str )
        {
            sha1_encoder sh;
            sh.input( &str.front(), str.size() );
            sha1_hashcode hc;
            sh.result( (unsigned*)hc.hash );
            return hc;
        }

    private:

        /*
         *  Process the next 512 bits of the message
         */
        void process_message_block();

        /*
         *  Pads the current message block to 512 bits
         */
        void pad_message();

        /*
         *  Performs a circular left shift operation
         */
        inline unsigned circular_shift(int bits, unsigned word);

        unsigned H[5];                      // Message digest buffers

        unsigned Length_Low;                // Message length in bits
        unsigned Length_High;               // Message length in bits

        unsigned char Message_Block[64];    // 512-bit message blocks
        int Message_Block_Index;            // Index into message block array

        bool Computed;                      // Is the digest computed?
        bool Corrupted;                     // Is the message digest corruped?
    
};

} } } // namespace boost::rpc::detail


namespace boost { namespace rpc {
    struct sha1 {};

    template<>
    struct datastream<sha1>
    {
        void result( sha1_hashcode& r ) { sh.result((unsigned*)r.hash);                            }
        sha1_hashcode result()          { sha1_hashcode r; sh.result((unsigned*)r.hash); return r; }

        template<typename DATA>
        inline datastream& operator<<(const DATA& d)
        {
            BOOST_STATIC_ASSERT( boost::is_fundamental<DATA>::value );
            sh.input( (const char*)&d, sizeof(d) );
            return *this;
        }
        inline bool putc(char c)  { *this << c; return true; }    

        inline void write( const char* d, uint32_t s )
        {
            sh.input( d, s );
        }

        private:
            detail::sha1_encoder sh;
    };
    inline void hash_sha1( const char* d, uint32_t s, sha1_hashcode& hc )
    {
        datastream<sha1> sh; sh.write(d,s); sh.result(hc);
    }
    inline sha1_hashcode hash_sha1( const char* d, uint32_t s )
    {
        datastream<sha1> sh; sh.write(d,s); 
        return sh.result();
    }
} } // namespace boost::rpc

BOOST_REFLECT_TYPEINFO(boost::rpc::sha1_hashcode)

#include <boost/rpc/datastream/sha1_impl.hpp>

#endif
