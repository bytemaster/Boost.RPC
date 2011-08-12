#ifndef _BOOST_RPC_RAW_HPP_
#define _BOOST_RPC_RAW_HPP_
#include <boost/reflect/reflect.hpp>

#include <boost/rpc/varint.hpp>
#include <boost/rpc/required.hpp>
#include <boost/rpc/errors.hpp>
#include <boost/cmt/log/log.hpp>
#include <boost/rpc/base64.hpp>
#include <sstream>
#include <iostream>
#include <map>
#include <iomanip>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>

namespace boost { namespace rpc { namespace raw {

    template<typename Stream, typename T>
    inline void pack( Stream& s, const T& v );
    template<typename Stream, typename T>
    inline void unpack( Stream& s, T& v );
    template<typename Stream, typename T, typename Alloc, template<typename,typename> class Container>
    inline void pack( Stream& s, const Container<T,Alloc>& value );
    template<typename Stream, typename T, typename Alloc, template<typename,typename> class Container>
    inline void unpack( Stream& s, Container<T,Alloc>& value );
    template<typename Stream, typename Key, typename Value>
    void pack( Stream& s, const std::map<Key,Value>& value );
    template<typename Stream, typename Key, typename Value>
    void unpack( Stream& s, std::map<Key,Value>& value );

    template<typename Stream> inline void pack( Stream& s, const signed_int& v ) {
      uint32_t val = (v.value<<1) ^ (v.value>>31);
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.put(b);
      } while( val );
    }
    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v ) {
      uint64_t val = v.value;
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.put(b);
      }while( val );
    }


    template<typename Stream> inline void pack( Stream& s, const bool& v ) { pack( s, uint8_t(v) );     }
    template<typename Stream> inline void pack( Stream& s, const char* v ) { pack( s, std::string(v) ); }

    template<typename Stream, typename T> 
    inline void pack( Stream& s, const required<T>& v ) { pack( s, *v); }

    template<typename Stream, typename T> 
    inline void pack( Stream& s, const boost::optional<T>& v ) {
      pack( s, bool(v) );
      if( v ) 
        pack( s, *v );
    }
    template<typename Stream> inline void pack( Stream& s, const std::vector<char>& value ) { 
      pack( s, unsigned_int(value.size()) );
      if( value.size() )
        s.write( &value.front(), value.size() );
    }
    template<typename Stream> inline void pack( Stream& s, const std::string& v )  {
      pack( s, unsigned_int(v.size()) );     
      if( v.size() )
        s.write( v.c_str(), v.size() );
    }

    template<typename Stream> inline void unpack( Stream& s, signed_int& vi ) {
      uint32_t v = 0; char b = 0; int by = 0;
      do {
        s.get(b);
        v |= uint32_t(uint8_t(b) & 0x7f) << by;
        by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
      vi.value = v&0x01 ? vi.value : -vi.value;
      vi.value = -vi.value;
    }
    template<typename Stream> inline void unpack( Stream& s, unsigned_int& vi ) {
      uint64_t v = 0; char b = 0; uint8_t by = 0;
      do {
          s.get(b);
          v |= uint32_t(uint8_t(b) & 0x7f) << by;
          by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = v;
    }
    template<typename Stream> inline void unpack( Stream& s, bool& v ) {
      uint8_t b; 
      unpack( s, b ); 
      v=b;    
    }
    template<typename Stream> inline void unpack( Stream& s, std::vector<char>& value ) { 
      unsigned_int size; unpack( s, size );
      value.resize(size.value);
      if( value.size() )
        s.read( &value.front(), value.size() );
    }
    template<typename Stream> inline void unpack( Stream& s, std::string& v )  {
      std::vector<char> tmp; unpack(s,tmp);
      v = std::string(tmp.begin(),tmp.end());
    }

    template<typename Stream, typename T>
    void unpack( const Stream& s, boost::optional<T>& v ) {
      bool b; unpack( s, b ); 
      if( b ) { v = T(); unpack( s, *v ); }
    }
    template<typename Stream, typename T>
    void unpack( const Stream& s, required<T>& v ) {
      v = T(); unpack( s, *v );
    }
    template<typename Stream, typename T, typename Alloc, template<typename,typename> class Container>
    void unpack( const Stream& s, Container<T,Alloc>& value );
    template<typename Stream, typename Key, typename Value>
    void unpack( const Stream& s, std::map<Key,Value>& value );
    template<typename Stream, typename Key, typename Value>
    void unpack( const Stream& s, std::pair<Key,Value>& value );
    template<typename Stream, typename Value>
    void unpack( const Stream& s, std::map<std::string,Value>& val );

    namespace detail {
    
      template<typename Stream, typename Class>
      struct pack_object_visitor {
        pack_object_visitor(const Class& _c, Stream& _s)
        :c(_c),s(_s){}

        /**
            VC++ does not understand the difference between:
            optional<T> Class::* and T Class::*
        */
        template<typename T>
        inline void pack_helper( const T& v, const char* name )const {
          boost::rpc::raw::pack( s, v ); 
        }
        template<typename T>
        inline void pack_helper( const boost::optional<T>& v, const char* name )const {
          if( !!v ) boost::rpc::raw::pack( s, *v ); 
        }
        template<typename T, typename C, T(C::*p)>
        inline void operator()( const char* name )const {
          pack_helper( c.*p, name );
        }
        private:            
          const Class& c;
          Stream&      s;
      };

      template<typename Stream, typename Class>
      struct unpack_object_visitor {
        unpack_object_visitor(Class& _c, Stream& _s)
        :c(_c),s(_s){}

        template<typename T, typename C, T(C::*p)>
        inline void operator()( const char* name )const {
          boost::rpc::raw::unpack( s, c.*p );
        }
        private:            
          Class&  c;
          Stream& s;
      };

      template<typename Stream>
      struct pack_sequence {
         pack_sequence( Stream& _s ):s(_s){}
         template<typename T>
         void operator() ( const T& v )const { boost::rpc::raw::pack(s,v); }
         Stream&    s;
      };

      template<typename Stream>
      struct unpack_sequence {
         unpack_sequence( Stream& _s ):s(_s){}
         template<typename T>
         void operator() ( T& v )const { boost::rpc::raw::unpack(s,v); }
         Stream&  s;
      };


      template<typename IsReflected=boost::false_type>
      struct if_reflected {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { 
          BOOST_STATIC_ASSERT( boost::is_pod<T>::value );
          s.write( (char*)&v, sizeof(v) );   
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { 
          BOOST_STATIC_ASSERT( boost::is_pod<T>::value );
          s.read( (char*)&v, sizeof(v) );   
        }
      };
      template<>
      struct if_reflected<boost::true_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { 
          boost::reflect::reflector<T>::visit( pack_object_visitor<Stream,T>( v, s ) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { 
          boost::reflect::reflector<T>::visit( unpack_object_visitor<Stream,T>( v, s ) );
        }
      };

      template<typename IsFusionSeq> 
      struct if_fusion_seq {
        template<typename Stream, typename T> 
        inline static void pack( Stream& s, const T& v ) {
          pack_sequence<Stream> pack_vector( s );
          boost::fusion::for_each( v, pack_vector );
        }
        template<typename Stream, typename T> 
        inline static void unpack( Stream& s, T& v ) {
          unpack_sequence<Stream> unpack_vector(s);
          boost::fusion::for_each( v, unpack_vector );
        }
      };

      template<> 
      struct if_fusion_seq<boost::mpl::false_> {
        template<typename Stream, typename T> 
        inline static void pack( Stream& s, const T& v ) {
          if_reflected<typename boost::reflect::reflector<T>::is_defined>::pack(s,v);
        }
        template<typename Stream, typename T> 
        inline static void unpack( Stream& s, T& v ) {
          if_reflected<typename boost::reflect::reflector<T>::is_defined>::unpack(s,v);
        }
      };
    } // namesapce detail

    

    template<typename Stream, typename T, typename Alloc, template<typename,typename> class Container>
    inline void pack( Stream& s, const Container<T,Alloc>& value ) {
      pack( s, unsigned_int(value.size()) );
      typename Container<T,Alloc>::const_iterator itr = value.begin();
      typename Container<T,Alloc>::const_iterator end = value.end();
      while( itr != end ) {
        boost::rpc::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T, typename Alloc, template<typename,typename> class Container>
    inline void unpack( Stream& s, Container<T,Alloc>& value ) {
      unsigned_int size; unpack( s, size );
      value.resize(size.value);
      typename Container<T,Alloc>::iterator itr = value.begin();
      typename Container<T,Alloc>::iterator end = value.end();
      while( itr != end ) {
        boost::rpc::raw::unpack( s, *itr );
        ++itr;
      }
    }

    // support for pair!
    template<typename Stream, typename Key, typename Value>
    inline void pack( Stream& s, const std::pair<Key,Value>& val ) {
      boost::rpc::raw::pack( s, val.first );
      boost::rpc::raw::pack( s, val.second );
    }
    // support for pair!
    template<typename Stream, typename Key, typename Value>
    void unpack( Stream& s, std::pair<Key,Value>& val ) {
      boost::rpc::raw::unpack( s, val.first );
      boost::rpc::raw::unpack( s, val.second );
    }


    // support arbitrary key/value containers as an array of pairs
    template<typename Stream, typename Key, typename Value>
    void pack( Stream& s, const std::map<Key,Value>& value ) {
      pack( s, unsigned_int(value.size()) );
      typename std::map<Key,Value>::const_iterator itr = value.begin();
      typename std::map<Key,Value>::const_iterator end = value.end();
      while( itr != end ) {
        boost::rpc::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename Key, typename Value>
    inline void unpack( Stream& s, std::map<Key,Value>& value ) {
      unsigned_int size; unpack( s, size );
      value.clear();
      for( uint32_t i = 0; i < size.value; ++i ) {
        Key k; 
        boost::rpc::raw::unpack(s,k);
        boost::rpc::raw::unpack(s,value[k]);
      }
    }
    template<typename Stream, typename T> 
    inline void pack( Stream& s, const T& v ) {
      detail::if_fusion_seq< typename boost::fusion::traits::is_sequence<T>::type >::pack(s,v);
    }
    template<typename Stream, typename T> 
    inline void unpack( Stream& s, T& v ) {
      detail::if_fusion_seq< typename boost::fusion::traits::is_sequence<T>::type >::unpack(s,v);
    }
    
} } } // namespace boost::rpc::raw

#endif // BOOST_RPC_RAW_HPP
