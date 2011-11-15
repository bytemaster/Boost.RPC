#ifndef _BOOST_RPC_JSON_VALUE_IO_HPP_
#define _BOOST_RPC_JSON_VALUE_IO_HPP_
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
#include <boost/rpc/json/value.hpp>

namespace boost { namespace rpc { namespace json {

    template<typename T> 
    void pack( boost::rpc::json::value& jsv, const T& v ); 
    template<typename T> 
    void unpack( const boost::rpc::json::value& jsv, T& v ); 


    inline void pack( boost::rpc::json::value& jsv, const boost::rpc::json::value& v ) { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const bool& v )         { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const float& v )        { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const double& v )       { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const uint8_t& v )      { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const uint16_t& v )     { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const uint32_t& v )     { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const uint64_t& v )     { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const int8_t& v )       { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const int16_t& v )      { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const int32_t& v )      { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const int64_t& v )      { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const signed_int& v )   { jsv = boost::rpc::json::value((int64_t)v.value); }
    inline void pack( boost::rpc::json::value& jsv, const unsigned_int& v ) { jsv = boost::rpc::json::value((uint64_t)v.value); }
    inline void pack( boost::rpc::json::value& jsv, const std::string& v )  { jsv = v; }
    inline void pack( boost::rpc::json::value& jsv, const char* v )         { jsv = v; }

    void pack( boost::rpc::json::value& jsv, const std::vector<char>& value );
    template<typename T>
    void pack( boost::rpc::json::value& jsv, const boost::optional<T>& v );
    template<typename T>
    void pack( boost::rpc::json::value& jsv, const required<T>& v );
    template<typename T, typename Alloc, template<typename,typename> class Container>
    void pack( boost::rpc::json::value& jsv, const Container<T,Alloc>& value );
    template<typename T, class Compare, typename Alloc, template<typename,class,typename> class Container>
    void pack( boost::rpc::json::value& jsv, const Container<T,Compare,Alloc>& value );
    template<typename Key, typename Value>
    void pack( boost::rpc::json::value& jsv, const std::map<Key,Value>& value );
    template<typename Key, typename Value>
    void pack( boost::rpc::json::value& jsv, const std::pair<Key,Value>& value );
    template<typename Value>
    void pack( boost::rpc::json::value& jsv, const std::map<std::string,Value>& value );


    template<typename T> 
    void unpack( const boost::rpc::json::value& jsv, const T& v ); 
    template<typename T> 
    void unpack( const boost::rpc::json::value& jsv, T& v ); 
    void unpack( const boost::rpc::json::value& jsv, bool& v );
    void unpack( const boost::rpc::json::value& jsv, float& v );
    void unpack( const boost::rpc::json::value& jsv, double& v );
    void unpack( const boost::rpc::json::value& jsv, uint8_t& v );
    void unpack( const boost::rpc::json::value& jsv, uint16_t& v );
    void unpack( const boost::rpc::json::value& jsv, uint32_t& v );
    void unpack( const boost::rpc::json::value& jsv, uint64_t& v );
    void unpack( const boost::rpc::json::value& jsv, int8_t& v );
    void unpack( const boost::rpc::json::value& jsv, int16_t& v );
    void unpack( const boost::rpc::json::value& jsv, int32_t& v );
    void unpack( const boost::rpc::json::value& jsv, int64_t& v );
    void unpack( const boost::rpc::json::value& jsv, signed_int& v );
    void unpack( const boost::rpc::json::value& jsv, unsigned_int& v );
    void unpack( const boost::rpc::json::value& jsv, std::string& v );
    void unpack( const boost::rpc::json::value& jsv, std::vector<char>& value );
    template<typename T>
    void unpack( const boost::rpc::json::value& jsv, boost::optional<T>& v );
    template<typename T>
    void unpack( const boost::rpc::json::value& jsv, required<T>& v );
    template<typename T, typename Alloc, template<typename,typename> class Container>
    void unpack( const boost::rpc::json::value& jsv, Container<T,Alloc>& value );
    template<typename T, class Compare, typename Alloc, template<typename,class,typename> class Container>
    void unpack( const boost::rpc::json::value& jsv, Container<T,Compare,Alloc>& value );
    template<typename Key, typename Value>
    void unpack( const boost::rpc::json::value& jsv, std::map<Key,Value>& value );
    template<typename Key, typename Value>
    void unpack( const boost::rpc::json::value& jsv, std::pair<Key,Value>& value );
    template<typename Value>
    void unpack( const boost::rpc::json::value& jsv, std::map<std::string,Value>& val );

    namespace detail {
      template<typename Class>
      struct pack_object_visitor {
        pack_object_visitor(const Class& _c, boost::rpc::json::object& _val)
        :c(_c),obj(_val){}

        /**
        VC++ does not understand the difference of return types, so an extra layer is needed.
        */
        template<typename T>
        inline void pack_helper( const T& v, const char* name )const {
          boost::rpc::json::pack( obj[name], v ); 
        }
        template<typename T>
        inline void pack_helper( const boost::optional<T>& v, const char* name )const {
          if( !!v ) {
            boost::rpc::json::pack( obj[name], *v ); 
          }
        }
        template<typename T,typename C, T(C::*p)>
        inline void operator()( const char* name )const {
          pack_helper( c.*p, name );
        }

        private:            
          const Class&              c;
          boost::rpc::json::object& obj;
      };

      template<typename Class>
      struct unpack_object_visitor  {
        unpack_object_visitor(Class& _c, const boost::rpc::json::object& _val)
        :c(_c),obj(_val){}

        template<typename T,typename C, T(C::*p)>
        void operator()( const char* name )const {
           if( obj.contains(name) )
               boost::rpc::json::unpack( obj[name], c.*p );
        }
        Class&                          c;
        const boost::rpc::json::object& obj;
      };

      struct pack_sequence {
         pack_sequence( boost::rpc::json::array& _a ):a(_a){}

         boost::rpc::json::array&    a;
         
         template<typename T>
         void operator() ( const T& v )const {
            a.push_back( boost::rpc::json::value());
            boost::rpc::json::pack(a.back(),v);
         }
      };
      struct unpack_sequence {
         unpack_sequence( boost::rpc::json::array::const_iterator& _a ):a(_a){}

         boost::rpc::json::array::const_iterator&  a;
         
         template<typename T>
         void operator() ( T& v )const {
            if( a != boost::rpc::json::array::const_iterator() ) {
                boost::rpc::json::unpack(*a,v);
                ++a;
            }
         }
      };

      template<bool IsFusionSeq> struct if_fusion_seq {
        template<typename T> 
        inline static void pack( boost::rpc::json::value& jsv, const T& v ) {
            jsv = boost::rpc::json::array();
            pack_sequence pack_vector( (boost::rpc::json::array&)jsv );
            boost::fusion::for_each( v, pack_vector );
        }
        template<typename T> 
        inline static void unpack( const boost::rpc::json::value& jsv, T& v ) {
            boost::rpc::json::array::const_iterator itr = boost::get<boost::rpc::json::array>(jsv.val).begin();
            unpack_sequence unpack_vector(itr);
            boost::fusion::for_each( v, unpack_vector );
        }
      };

      template<typename IsReflected=boost::false_type>
      struct if_reflected {
        template<typename T>
        static inline void pack( boost::rpc::json::value& s, const T& v ) { 
          std::stringstream ss; ss << v;
          // TO BASE 64
          boost::rpc::json::pack(s,base64_encode((unsigned char const*)ss.str().c_str(),ss.str().size()));
        }
        template<typename T>
        static inline void unpack( const boost::rpc::json::value& s, T& v ) { 
          std::string str;
          boost::rpc::json::unpack(s,str);
          std::stringstream ss(base64_decode(str)); 
          ss >> v;
        }
      };
      template<>
      struct if_reflected<boost::true_type> {
        template<typename T>
        static inline void pack( boost::rpc::json::value& jsv, const T& v ) { 
              jsv = boost::rpc::json::object();
              detail::pack_object_visitor<T> pov(v,jsv);
              boost::reflect::reflector<T>::visit(pov);
        }
        template<typename T>
        static inline void unpack( const boost::rpc::json::value& jsv, T& v ) { 
              detail::unpack_object_visitor<T> pov(v,jsv );
              boost::reflect::reflector<T>::visit(pov);
        }
      };

      template<> struct if_fusion_seq<false> {
          template<typename T> 
          inline static void pack( boost::rpc::json::value& jsv, const T& v ) {
              if_reflected<typename boost::reflect::reflector<T>::is_defined>::pack(jsv,v);
          }
          template<typename T> 
          inline static void unpack( const boost::rpc::json::value& jsv, T& v ) {
              if_reflected<typename boost::reflect::reflector<T>::is_defined>::unpack(jsv,v);
          }
      };




      /*

      template<> struct if_fusion_seq<false> {
          template<typename T> 
          inline static void pack( boost::rpc::json::value& jsv, const T& v ) {
              jsv = boost::rpc::json::object();
              detail::pack_object_visitor<T> pov(v,jsv);
              boost::reflect::reflector<T>::visit(pov);
          }
          template<typename T> 
          inline static void unpack( const boost::rpc::json::value& jsv, T& v ) {
              detail::unpack_object_visitor<T> pov(v,jsv );
              boost::reflect::reflector<T>::visit(pov);
          }
      };
      */
    } // namesapce detail

    
    inline void unpack( const boost::rpc::json::value& jsv, bool& v )         { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, float& v )        { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, double& v )       { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, uint8_t& v )      { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, uint16_t& v )     { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, uint32_t& v )     { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, uint64_t& v )     { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, int8_t& v )       { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, int16_t& v )      { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, int32_t& v )      { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, int64_t& v )      { v = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, signed_int& v )   { v.value = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, unsigned_int& v ) { v.value = jsv; }
    inline void unpack( const boost::rpc::json::value& jsv, std::string& v )  { v = (const std::string&)jsv; }

    template<typename T> 
    inline void pack( boost::rpc::json::value& jsv, const T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::pack(jsv,v);
    }

    
    template<typename T> 
    inline void pack( boost::rpc::json::value& jsv, const boost::optional<T>& v ) {
        boost::rpc::json::pack( jsv, *v );
    }

    template<typename T>
    inline void pack( boost::rpc::json::value& jsv, const required<T>& v ) {
        if( !v ) BOOST_THROW_EXCEPTION( boost::rpc::required_field_not_set() );
        boost::rpc::json::pack( jsv, *v );
    }
    template<typename T>
    inline void unpack( const boost::rpc::json::value& jsv, required<T>& v ) {
        v = T();
        boost::rpc::json::unpack( jsv, *v );
    }
    template<typename T>
    inline void unpack( const boost::rpc::json::value& jsv, optional<T>& v ) {
        v = T();
        boost::rpc::json::unpack( jsv, *v );
    }


    template<typename T, typename Alloc, template<typename,typename> class Container>
    inline void pack( boost::rpc::json::value& jsv, const Container<T,Alloc>& value ) {
        jsv = boost::rpc::json::array();
        boost::rpc::json::array& a = jsv;
        a.resize(value.size());
        typename Container<T,Alloc>::const_iterator itr = value.begin();
        typename Container<T,Alloc>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            boost::rpc::json::pack( a[i], *itr );
            ++itr;
            ++i;
        }
    }
    template<typename T, class Compare, typename Alloc, template<typename,class,typename> class Container>
    inline void pack( boost::rpc::json::value& jsv, const Container<T,Compare,Alloc>& value ) {
        jsv = boost::rpc::json::array();
        boost::rpc::json::array& a = jsv;
        a.resize(value.size());
        typename Container<T,Compare,Alloc>::const_iterator itr = value.begin();
        typename Container<T,Compare,Alloc>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            boost::rpc::json::pack( a[i], *itr );
            ++itr;
            ++i;
        }
    }

    template<typename T, typename Alloc, template<typename,typename> class Container>
    inline void unpack( const boost::rpc::json::value& jsv, Container<T,Alloc>& value ) {
        const boost::rpc::json::array& a = jsv;
        value.reserve( a.size() );
        for( uint32_t i = 0; i < a.size(); ++i ) {
            T v;
            boost::rpc::json::unpack( a[i], v );
            value.push_back(v);
        }
    }
    template<typename T, class Compare, typename Alloc, template<typename,class,typename> class Container>
    inline void unpack( const boost::rpc::json::value& jsv, Container<T,Compare,Alloc>& value ) {
        const boost::rpc::json::array& a = jsv;
        value.resize( a.size() );
        typename Container<T,Compare,Alloc>::iterator itr = value.begin();
        typename Container<T,Compare,Alloc>::iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            boost::rpc::json::unpack( a[i], *itr );
            ++itr;
            ++i;
        }
    }

    // support for pair!
    template<typename Key, typename Value>
    inline void pack( boost::rpc::json::value& jsv, const std::pair<Key,Value>& val ) {
        jsv = boost::rpc::json::object();
        boost::rpc::json::pack( jsv[std::string("first")], val.first );
        boost::rpc::json::pack( jsv[std::string("second")], val.second );
    }
    // support for pair!
    template<typename Key, typename Value>
    void unpack( const boost::rpc::json::value& jsv, std::pair<Key,Value>& val ) {
        boost::rpc::json::unpack( jsv[std::string("first")], val.first );
        boost::rpc::json::unpack( jsv[std::string("second")], val.second );
    }


    // support arbitrary key/value containers as an array of pairs
    template<typename Key, typename Value>
    void pack( boost::rpc::json::value& jsv, const std::map<Key,Value>& value ) {
        jsv = boost::rpc::json::array();
        boost::rpc::json::array& a = jsv;
        a.resize(value.size());
        typename std::map<Key,Value>::const_iterator itr = value.begin();
        typename std::map<Key,Value>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            boost::rpc::json::pack( a[i], *itr );
            ++itr;
            ++i;
        }
    }

    template<typename Key, typename Value>
    inline void unpack( const boost::rpc::json::value& jsv, std::map<Key,Value>& value ) {
        const boost::rpc::json::array& a = jsv;
        value.clear();
        for( uint32_t i = 0; i < a.size(); ++i ) {
            std::pair<Key,Value> p;
            boost::rpc::json::unpack( a[i], p );
            value[p.first] = p.second;
        }
    }

    inline void pack( boost::rpc::json::value& jsv, const std::vector<char>& data ) {
       if( data.size() ) { pack( jsv, base64_encode((unsigned char*)&data.front(),data.size())); } 
    }
    inline void unpack( const boost::rpc::json::value& jsv, std::vector<char>& data ) {
        data.clear();
        std::string d = base64_decode(jsv);
        data.insert(data.begin(),d.begin(),d.end());
    }


    // pack map<string,T> as a JSON Object
    template<typename Value>
    void pack( boost::rpc::json::value& jsv, const std::map<std::string,Value>& val ) {
        jsv = boost::rpc::json::object();
        boost::rpc::json::object& o = jsv;
        o.reserve(val.size());
        typename std::map<std::string,Value>::const_iterator itr = val.begin();
        typename std::map<std::string,Value>::const_iterator end = val.end();
        while( itr != end ) {
            boost::rpc::json::pack( o[itr->first], itr->second );
            ++itr;
        }
    }
    template<typename Value>
    void unpack( const boost::rpc::json::value& jsv, std::map<std::string,Value>& val ) {
        const boost::rpc::json::object& o = jsv;
        val.clear();
        for( uint32_t i = 0; i < o.keys.size(); ++i ) {
            boost::rpc::json::unpack( o.keys[i].val, val[o.keys[i].key] );
        }
    }
    template<typename T> 
    inline void unpack( const boost::rpc::json::value& jsv, T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::unpack(jsv,v);
    }

    template<typename T>
    std::ostream& to_json( std::ostream& os, const T& v ) {
        boost::rpc::json::value jsv; 
        pack( jsv, v );
        write(os,jsv);
        return os;
    }

    template<typename T>
    std::string to_string( const T& v, bool pretty = false ) {
        std::stringstream ss;
        boost::rpc::json::value jsv; 
        pack( jsv, v );
        write(ss,jsv,pretty);
        return ss.str();
    }

    template<typename T>
    struct to_json_helper {
        inline to_json_helper( const T& v):val(v){} 
        operator std::string()const;
        const T&  val;
    };
    template<typename T>
    to_json_helper<T> to_json( const T& v ) {
        return v;
    }
    template<typename T> 
    inline std::ostream& operator<<(std::ostream& os, const to_json_helper<T>& h ) {
        return to_json( os, h.val);
    }
    template<typename T> 
    to_json_helper<T>::operator std::string()const {
        std::stringstream ss; ss << *this; 
        return ss.str();
    }

    template<typename T>
    void from_json( const std::string& j, T& val ) {
        boost::rpc::json::value v;
        boost::rpc::json::read( j, v );
        unpack( v, val );
    }
    
} // json 

namespace raw {
    template<typename Stream>
    inline void pack( Stream& s, const std::string& v );
    template<typename Stream>
    inline void unpack( Stream& s, std::string& v );

    template<typename Stream> inline void pack( Stream& s, const json::value& v ) { 
      std::stringstream ss; 
      write( ss, v );
      pack( s, ss.str() );
   }
    template<typename Stream> inline void unpack( Stream& s, json::value& v ) { 
      std::string str;
      unpack(s,str);
      read( str, v );
   }
} } } // namespace boost::rpc::raw

#endif // BOOST_RPC_JSON_VALUE_IO_HPP
