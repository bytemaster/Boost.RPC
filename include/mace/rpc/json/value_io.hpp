#ifndef _MACE_RPC_JSON_VALUE_IO_HPP_
#define _MACE_RPC_JSON_VALUE_IO_HPP_
#include <mace/reflect/reflect.hpp>
#include <mace/rpc/base64.hpp>
#include <sstream>
#include <iostream>
#include <map>
#include <iomanip>

#include <boost/optional.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <mace/rpc/json/value.hpp>

namespace mace { namespace rpc { namespace json { namespace io {


    template<typename T, typename Filter> 
    void pack( Filter&, json::value& jsv, const T& v ); 

    template<typename T, typename Filter> 
    void unpack( Filter&, const json::value& jsv, T& v ); 


    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const json::value& v )  { jsv = v; }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const bool& v )         { jsv = f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const float& v )        { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const double& v )       { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const uint8_t& v )      { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const uint16_t& v )     { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const uint32_t& v )     { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const uint64_t& v )     { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const int8_t& v )       { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const int16_t& v )      { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const int32_t& v )      { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const int64_t& v )      { jsv = (double)f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const std::string& v )  { jsv = f(v); }
    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const char* v )         { jsv = f(v); }

    template<typename Filter>
    void pack( Filter& c, json::value& jsv, const std::vector<char>& value );
    template<typename T, typename Filter>
    void pack( Filter& c, json::value& jsv, const boost::optional<T>& v );
    template<typename T, typename Filter>
    void pack( Filter& c, json::value& jsv, const std::vector<T>& value );
    template<typename T, typename Filter>
    void pack( Filter& c, json::value& jsv, const std::list<T>& value );

    template<typename T, typename Filter>
    void pack( Filter& c, json::value& jsv, const std::set<T>& value );
    template<typename Key, typename Value, typename Filter>
    void pack( Filter& c, json::value& jsv, const std::map<Key,Value>& value );
    template<typename Key, typename Value, typename Filter>
    void pack( Filter& c, json::value& jsv, const std::pair<Key,Value>& value );
    template<typename Value, typename Filter>
    void pack( Filter& c, json::value& jsv, const std::map<std::string,Value>& value );


    template<typename Filter>
    inline void unpack( Filter& c, const json::value& jsv, json::value& v ) { v = jsv; }
    template<typename T, typename Filter>
    void unpack( Filter& c, const json::value& jsv, const T& v ); 
    template<typename T, typename Filter> 
    void unpack( Filter& c, const json::value& jsv, T& v ); 
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, bool& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, float& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, double& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, uint8_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, uint16_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, uint32_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, uint64_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, int8_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, int16_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, int32_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, int64_t& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::string& v );
    template<typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::vector<char>& value );
    template<typename T, typename Filter>
    void unpack( Filter& c, const json::value& jsv, boost::optional<T>& v );
    template<typename T, typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::vector<T>& value );
    template<typename T, typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::list<T>& value );

    template<typename T, typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::set<T>& value );
    template<typename Key, typename Value, typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::map<Key,Value>& value );
    template<typename Key, typename Value, typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::pair<Key,Value>& value );
    template<typename Value, typename Filter>
    void unpack( Filter& c, const json::value& jsv, std::map<std::string,Value>& val );

    namespace detail {
      template<typename Class, typename Filter>
      struct pack_object_visitor {
        pack_object_visitor(Filter& _f, const Class& _c, json::object& _val)
        :f(_f),c(_c),obj(_val){}

        /**
        VC++ does not understand the difference of return types, so an extra layer is needed.
        */
        template<typename T>
        inline void pack_helper( const T& v, const char* name )const {
          json::io::pack( f, obj[name], f(v) ); 
        }
        template<typename T>
        inline void pack_helper( const boost::optional<T>& v, const char* name )const {
          if( !!v ) {
            json::io::pack( f, obj[name], f(*v) ); 
          }
        }
        template<typename T, T  p>
        inline void operator()( const char* name )const {
          pack_helper( c.*p, name );
        }

        private:            
          Filter&       f;
          const Class&  c;
          json::object& obj;
      };

      template<typename Class, typename Filter>
      struct unpack_object_visitor  {
        unpack_object_visitor(Filter& _f, Class& _c, const json::object& _val)
        :f(_f),c(_c),obj(_val){}

        template<typename T, T p>
        void operator()( const char* name )const {
           if( obj.contains(name) )
               json::io::unpack( f, obj[name], c.*p );
        }
        Filter&             f;
        Class&              c;
        const json::object& obj;
      };

      template<typename Filter>
      struct pack_sequence {
         pack_sequence( Filter& _f, json::array& _a ):f(_f),a(_a){}

         Filter&         f;
         json::array&    a;
         
         template<typename T>
         void operator() ( const T& v )const {
            a.push_back( json::value());
            json::io::pack(f,a.back(),f(v));
         }
      };

      template<typename Filter>
      struct unpack_sequence {
         unpack_sequence( Filter& _f, json::array::const_iterator& _a ):f(_f),a(_a){}

         Filter&                       f;
         json::array::const_iterator&  a;
         
         template<typename T>
         void operator() ( T& v )const {
            if( a != json::array::const_iterator() ) {
                f( *a, v );
                //json::io::unpack(f,*a,v);
                ++a;
            }
         }
      };

      template<bool IsFusionSeq> struct if_fusion_seq {
        template<typename T,typename Filter> 
        inline static void pack( Filter& f, json::value& jsv, const T& v ) {
            jsv = json::array();
            pack_sequence<Filter> pack_vector(f, (json::array&)jsv );
            boost::fusion::for_each( v, pack_vector );
        }
        template<typename T,typename Filter> 
        inline static void unpack( Filter& f, const json::value& jsv, T& v ) {
            json::array::const_iterator itr = boost::get<json::array>(jsv.val).begin();
            unpack_sequence<Filter> unpack_vector(f,itr);
            boost::fusion::for_each( v, unpack_vector );
        }
      };

      template<typename IsReflected=boost::false_type>
      struct if_reflected {
        template<typename T,typename Filter>
        static inline void pack( Filter& f,json::value& s, const T& v ) { 
          std::stringstream ss; ss << v;
          // TO BASE 64
          json::io::pack(f,s,f(mace::rpc::base64_encode((unsigned char const*)ss.str().c_str(),ss.str().size())));
        }
        template<typename T,typename Filter>
        static inline void unpack( Filter& f, const json::value& s, T& v ) { 
          std::string str;
          json::io::unpack(f,s,str);
          std::stringstream ss(mace::rpc::base64_decode(str)); 
          ss >> v;
        }
      };
      template<>
      struct if_reflected<boost::true_type> {
        template<typename T,typename Filter>
        static inline void pack( Filter& f, json::value& jsv, const T& v ) { 
              jsv = json::object();
              detail::pack_object_visitor<T,Filter> pov(f,f(v),jsv);
              mace::reflect::reflector<T>::visit(pov);
        }
        template<typename T,typename Filter>
        static inline void unpack( Filter& f, const json::value& jsv, T& v ) { 
              detail::unpack_object_visitor<T,Filter> pov(f,v,jsv );
              mace::reflect::reflector<T>::visit(pov);
        }
      };

      template<> struct if_fusion_seq<false> {
          template<typename T,typename Filter> 
          inline static void pack( Filter& f, json::value& jsv, const T& v ) {
              if_reflected<typename mace::reflect::reflector<T>::is_defined>::pack(f,jsv,f(v));
          }
          template<typename T,typename Filter> 
          inline static void unpack( Filter& f, const json::value& jsv, T& v ) {
              if_reflected<typename mace::reflect::reflector<T>::is_defined>::unpack(f,jsv,v);
          }
      };




      /*

      template<> struct if_fusion_seq<false> {
          template<typename T> 
          inline static void pack( json::value& jsv, const T& v ) {
              jsv = json::object();
              detail::pack_object_visitor<T> pov(v,jsv);
              mace::reflect::reflector<T>::visit(pov);
          }
          template<typename T> 
          inline static void unpack( const json::value& jsv, T& v ) {
              detail::unpack_object_visitor<T> pov(v,jsv );
              mace::reflect::reflector<T>::visit(pov);
          }
      };
      */
    } // namesapce detail

    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, bool& v )         { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, float& v )        { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, double& v )       { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, uint8_t& v )      { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, uint16_t& v )     { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, uint32_t& v )     { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, uint64_t& v )     { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, int8_t& v )       { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, int16_t& v )      { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, int32_t& v )      { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, int64_t& v )      { v = jsv; }
    template<typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, std::string& v )  { v = (const std::string&)jsv; }

    template<typename T, typename Filter> 
    inline void pack( Filter& f, json::value& jsv, const T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::pack(f,jsv,f(v));
    }

    
    template<typename T, typename Filter> 
    inline void pack( Filter& f, json::value& jsv, const boost::optional<T>& v ) {
        json::io::pack( f, jsv, f(*v) );
    }

    template<typename T, typename Filter>
    inline void unpack( Filter& f, const json::value& jsv, boost::optional<T>& v ) {
        v = T();
        json::io::unpack( f, jsv, *v );
    }


    template<typename T, typename Filter>
    inline void pack( Filter& f, json::value& jsv, const std::vector<T>& value ) {
        jsv = json::array();
        json::array& a = jsv;
        a.resize(value.size());
        typename std::vector<T>::const_iterator itr = value.begin();
        typename std::vector<T>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            json::io::pack( f, a[i], f(*itr) );
            ++itr;
            ++i;
        }
    }

    template<typename T, typename Filter>
    inline void pack( Filter& f, json::value& jsv, const std::list<T>& value ) {
        jsv = json::array();
        json::array& a = jsv;
        a.resize(value.size());
        typename std::list<T>::const_iterator itr = value.begin();
        typename std::list<T>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            json::io::pack( f, a[i], f(*itr) );
            ++itr;
            ++i;
        }
    }


    template<typename T, typename Filter>
    inline void pack( Filter& f, json::value& jsv, const std::set<T>& value ) {
        jsv = json::array();
        json::array& a = jsv;
        a.resize(value.size());
        typename std::set<T>::const_iterator itr = value.begin();
        typename std::set<T>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            json::io::pack( f, a[i], f(*itr) );
            ++itr;
            ++i;
        }
    }

    template<typename T, typename Filter>
    inline void unpack( Filter& f, const json::value& jsv, std::vector<T>& value ) {
        const json::array& a = jsv;
        value.reserve( a.size() );
        for( uint32_t i = 0; i < a.size(); ++i ) {
            T v;
            json::io::unpack( f, a[i], v );
            value.push_back(v);
        }
    }
    template<typename T, typename Filter>
    inline void unpack( Filter& f, const json::value& jsv, std::list<T>& value ) {
        const json::array& a = jsv;
        value.reserve( a.size() );
        for( uint32_t i = 0; i < a.size(); ++i ) {
            T v;
            json::io::unpack( f, a[i], v );
            value.push_back(v);
        }
    }
    template<typename T, typename Filter>
    inline void unpack( Filter& f, const json::value& jsv, std::set<T>& value ) {
        const json::array& a = jsv;
        typename std::set<T>::iterator itr = value.begin();
        typename std::set<T>::iterator end = value.end();
        for( uint32_t i = 0; i < a.size(); ++i ) {
            T v;
            json::io::unpack( f, a[i], v );
            value.insert(v);
        }
    }

    // support for pair!
    template<typename Key, typename Value, typename Filter>
    inline void pack( Filter& f, json::value& jsv, const std::pair<Key,Value>& val ) {
        jsv = json::object();
        json::io::pack( f, jsv[std::string("first")], f(val.first) );
        json::io::pack( f, jsv[std::string("second")], f(val.second) );
    }
    // support for pair!
    template<typename Key, typename Value, typename Filter>
    void unpack( Filter& f, const json::value& jsv, std::pair<Key,Value>& val ) {
        json::io::unpack( f, jsv[std::string("first")], val.first );
        json::io::unpack( f, jsv[std::string("second")], val.second );
    }


    // support arbitrary key/value containers as an array of pairs
    template<typename Key, typename Value, typename Filter>
    void pack( Filter& f, json::value& jsv, const std::map<Key,Value>& value ) {
        jsv = json::array();
        json::array& a = jsv;
        a.resize(value.size());
        typename std::map<Key,Value>::const_iterator itr = value.begin();
        typename std::map<Key,Value>::const_iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            json::io::pack( f, a[i], f(*itr) );
            ++itr;
            ++i;
        }
    }

    template<typename Key, typename Value, typename Filter>
    inline void unpack( Filter& f, const json::value& jsv, std::map<Key,Value>& value ) {
        const json::array& a = jsv;
        value.clear();
        for( uint32_t i = 0; i < a.size(); ++i ) {
            std::pair<Key,Value> p;
            json::io::unpack( f, a[i], p );
            value[p.first] = p.second;
        }
    }

    template<typename Filter>
    inline void pack( Filter& f, json::value& jsv, const std::vector<char>& data ) {
       if( data.size() ) { pack( f, jsv, f(mace::rpc::base64_encode((unsigned char*)&data.front(),data.size()))); } 
    }
    template<typename Filter>
    inline void unpack( Filter& f, const json::value& jsv, std::vector<char>& data ) {
        data.clear();
        std::string d = mace::rpc::base64_decode(jsv);
        data.insert(data.begin(),d.begin(),d.end());
    }


    // pack map<string,T> as a JSON Object
    template<typename Value,typename Filter>
    void pack( Filter& f, json::value& jsv, const std::map<std::string,Value>& val ) {
        jsv = json::object();
        json::object& o = jsv;
        o.reserve(val.size());
        typename std::map<std::string,Value>::const_iterator itr = val.begin();
        typename std::map<std::string,Value>::const_iterator end = val.end();
        while( itr != end ) {
            json::io::pack( f, o[itr->first], f(itr->second) );
            ++itr;
        }
    }
    template<typename Value, typename Filter>
    void unpack( Filter& f, const json::value& jsv, std::map<std::string,Value>& val ) {
        const json::object& o = jsv;
        val.clear();
        for( uint32_t i = 0; i < o.keys.size(); ++i ) {
            json::io::unpack( f, o.keys[i].val, val[o.keys[i].key] );
        }
    }
    template<typename T, typename Filter> 
    inline void unpack( Filter& f, const json::value& jsv, T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::unpack(f, jsv,v);
    }

    struct default_filter {
        template<typename T>
        inline void operator()( const json::value& j, T& v ) { 
          return json::io::unpack( *this, j, v ); 
        }

        template<typename T>
        inline const T& operator()( const T& v ) { return v; }
    };

    template<typename T>
    std::ostream& to_json( std::ostream& os, const T& v ) {
        default_filter f; 
        json::value jsv; 
        pack( f, jsv, v );
        json::write(os,jsv);
        return os;
    }

    template<typename T>
    std::string to_string( const T& v, bool pretty = false ) {
        default_filter f; 
        std::stringstream ss;
        json::value jsv; 
        pack( f, jsv, v );
        json::write(ss,jsv,pretty);
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
        default_filter f;
        json::value v;
        json::from_string( j, v );
        unpack( f, v, val );
    }


    template<typename T> 
    void pack( json::value& jsv, const T& v )  {
      default_filter f;
      pack( f, jsv, v );
    }

    template<typename T> 
    void unpack( const json::value& jsv, T& v ) {
      default_filter f;
      unpack( f, jsv, v );
    }
    
} } } } // mace::rpc::json::io

#endif // JSON_VALUE_IO_HPP
