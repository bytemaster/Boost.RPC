#ifndef _BOOST_RPC_JSON_HPP_
#define _BOOST_RPC_JSON_HPP_
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
#include <boost/json/value.hpp>
#include <boost/json/writer.hpp>
#include <boost/json/reader.hpp>

namespace boost { namespace rpc { namespace json {

    template<typename T> 
    void pack( boost::json::Value& jsv, const T& v ); 
    template<typename T> 
    void unpack( const boost::json::Value& jsv, T& v ); 
    void pack( boost::json::Value& jsv, const bool& v );
    void pack( boost::json::Value& jsv, const float& v );
    void pack( boost::json::Value& jsv, const double& v );
    void pack( boost::json::Value& jsv, const uint8_t& v );
    void pack( boost::json::Value& jsv, const uint16_t& v );
    void pack( boost::json::Value& jsv, const uint32_t& v );
    void pack( boost::json::Value& jsv, const uint64_t& v );
    void pack( boost::json::Value& jsv, const int8_t& v );
    void pack( boost::json::Value& jsv, const int16_t& v );
    void pack( boost::json::Value& jsv, const int32_t& v );
    void pack( boost::json::Value& jsv, const int64_t& v );
    void pack( boost::json::Value& jsv, const signed_int& v );
    void pack( boost::json::Value& jsv, const unsigned_int& v );
    void pack( boost::json::Value& jsv, const std::string& v );
    void pack( boost::json::Value& jsv, const char* v );
    void pack( boost::json::Value& jsv, const std::vector<char>& value );
    template<typename T>
    void pack( boost::json::Value& jsv, const boost::optional<T>& v );
    template<typename T>
    void pack( boost::json::Value& jsv, const required<T>& v );
    template<typename T, typename Alloc, template<typename,typename> class Container>
    void pack( boost::json::Value& jsv, const Container<T,Alloc>& value );
    template<typename Key, typename Value>
    void pack( boost::json::Value& jsv, const std::map<Key,Value>& value );
    template<typename Key, typename Value>
    void pack( boost::json::Value& jsv, const std::pair<Key,Value>& value );
    template<typename Value>
    void pack( boost::json::Value& jsv, const std::map<std::string,Value>& value );


    template<typename T> 
    void unpack( const boost::json::Value& jsv, const T& v ); 
    template<typename T> 
    void unpack( const boost::json::Value& jsv, T& v ); 
    void unpack( const boost::json::Value& jsv, bool& v );
    void unpack( const boost::json::Value& jsv, float& v );
    void unpack( const boost::json::Value& jsv, double& v );
    void unpack( const boost::json::Value& jsv, uint8_t& v );
    void unpack( const boost::json::Value& jsv, uint16_t& v );
    void unpack( const boost::json::Value& jsv, uint32_t& v );
    void unpack( const boost::json::Value& jsv, uint64_t& v );
    void unpack( const boost::json::Value& jsv, int8_t& v );
    void unpack( const boost::json::Value& jsv, int16_t& v );
    void unpack( const boost::json::Value& jsv, int32_t& v );
    void unpack( const boost::json::Value& jsv, int64_t& v );
    void unpack( const boost::json::Value& jsv, signed_int& v );
    void unpack( const boost::json::Value& jsv, unsigned_int& v );
    void unpack( const boost::json::Value& jsv, std::string& v );
    void unpack( const boost::json::Value& jsv, std::vector<char>& value );
    template<typename T>
    void unpack( const boost::json::Value& jsv, boost::optional<T>& v );
    template<typename T>
    void unpack( const boost::json::Value& jsv, required<T>& v );
    template<typename T, typename Alloc, template<typename,typename> class Container>
    void unpack( const boost::json::Value& jsv, Container<T,Alloc>& value );
    template<typename Key, typename Value>
    void unpack( const boost::json::Value& jsv, std::map<Key,Value>& value );
    template<typename Key, typename Value>
    void unpack( const boost::json::Value& jsv, std::pair<Key,Value>& value );
    template<typename Value>
    void unpack( const boost::json::Value& jsv, std::map<std::string,Value>& value );

    namespace detail {
      template<typename Class>
      struct pack_object_visitor {
        pack_object_visitor(const Class& _c, boost::json::Object& _val)
        :c(_c),obj(_val){}

        /**
        VC++ does not understand the difference of return types, so an extra layer is
        needed.
        */
        template<typename T>
        void pack_helper( const T& v, const char* name )const {
          obj.push_back( boost::json::Pair(name,boost::json::Value()) );
          boost::rpc::json::pack( obj.back().value_, v );
        }
        template<typename T>
        void pack_helper( const boost::optional<T>& v, const char* name )const {
          if( !!v ) {
            obj.push_back( boost::json::Pair(name,boost::json::Value()) );
            boost::rpc::json::pack( obj.back().value_, *v );
          }
        }
        template<typename T>
        void operator()( T (Class::*p), const char* name )const {
          pack_helper( c.*p, name );
        }

        private:            
          const Class&         c;
          boost::json::Object& obj;
      };

      template<typename Class>
      struct unpack_object_visitor  {
        unpack_object_visitor(Class& _c, const boost::json::Object& _val)
        :c(_c),obj(_val){}

        template<typename T>
        void operator()( T (Class::*p), const char* name )const {
           std::string n(name);
           for( uint32_t i = 0; i < obj.size(); ++i ) {
             if( obj[i].name_ == n ) {
                 boost::rpc::json::unpack( obj[i].value_, c.*p );
                 return;
             }
           }
        }
        Class&                     c;
        const boost::json::Object& obj;
      };

      struct pack_sequence {
         pack_sequence( boost::json::Array& _a ):a(_a){}

         boost::json::Array&    a;
         
         template<typename T>
         void operator() ( const T& v )const {
            a.push_back( boost::json::Value());
            boost::rpc::json::pack(a.back(),v);
         }
      };
      struct unpack_sequence {
         unpack_sequence( boost::json::Array::const_iterator& _a ):a(_a){}

         boost::json::Array::const_iterator&  a;
         
         template<typename T>
         void operator() ( T& v )const {
            if( a != boost::json::Array::const_iterator() ) {
                boost::rpc::json::unpack(*a,v);
                ++a;
            }
         }
      };

      template<bool IsFusionSeq> struct if_fusion_seq {
        template<typename T> 
        inline static void pack( boost::json::Value& jsv, const T& v ) {
            jsv = boost::json::Array();
            pack_sequence pack_vector(jsv.get_array());
            boost::fusion::for_each( v, pack_vector );
        }
        template<typename T> 
        inline static void unpack( const boost::json::Value& jsv, T& v ) {
            boost::json::Array::const_iterator itr = jsv.get_array().begin();
            unpack_sequence unpack_vector(itr);
            boost::fusion::for_each( v, unpack_vector );
        }
      };
      template<> struct if_fusion_seq<false> {
          template<typename T> 
          inline static void pack( boost::json::Value& jsv, const T& v ) {
              jsv = boost::json::Object();
              detail::pack_object_visitor<T> pov(v, jsv.get_obj());
              boost::reflect::reflector<T>::visit(pov);
          }
          template<typename T> 
          inline static void unpack( const boost::json::Value& jsv, T& v ) {
              detail::unpack_object_visitor<T> pov(v, jsv.get_obj());
              boost::reflect::reflector<T>::visit(pov);
          }
      };
    } // namesapce detail

    inline void pack( boost::json::Value& jsv, const bool& v )         { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const float& v )        { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const double& v )       { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const uint8_t& v )      { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const uint16_t& v )     { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const uint32_t& v )     { jsv = boost::json::Value((uint64_t)v); }
    inline void pack( boost::json::Value& jsv, const uint64_t& v )     { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const int8_t& v )       { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const int16_t& v )      { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const int32_t& v )      { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const int64_t& v )      { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const signed_int& v )   { jsv = boost::json::Value((int64_t)v.value); }
    inline void pack( boost::json::Value& jsv, const unsigned_int& v ) { jsv = boost::json::Value((uint64_t)v.value); }
    inline void pack( boost::json::Value& jsv, const std::string& v )  { jsv = boost::json::Value(v); }
    inline void pack( boost::json::Value& jsv, const char* v )         { jsv = boost::json::Value(std::string(v)); }
    
    inline void unpack( const boost::json::Value& jsv, bool& v )         { v = jsv.get_bool();           }
    inline void unpack( const boost::json::Value& jsv, float& v )        { v = (float)jsv.get_real();    }
    inline void unpack( const boost::json::Value& jsv, double& v )       { v = jsv.get_real();           }
    inline void unpack( const boost::json::Value& jsv, uint8_t& v )      { v = jsv.get_int();            }
    inline void unpack( const boost::json::Value& jsv, uint16_t& v )     { v = jsv.get_int();            }
    inline void unpack( const boost::json::Value& jsv, uint32_t& v )     { v = (uint32_t)jsv.get_int64();}
    inline void unpack( const boost::json::Value& jsv, uint64_t& v )     { v = jsv.get_uint64();         }
    inline void unpack( const boost::json::Value& jsv, int8_t& v )       { v = jsv.get_int();            }
    inline void unpack( const boost::json::Value& jsv, int16_t& v )      { v = jsv.get_int();            }
    inline void unpack( const boost::json::Value& jsv, int32_t& v )      { v = jsv.get_int();            }
    inline void unpack( const boost::json::Value& jsv, int64_t& v )      { v = jsv.get_int64();          }
    inline void unpack( const boost::json::Value& jsv, signed_int& v )   { v.value = (int32_t)jsv.get_int64();    }
    inline void unpack( const boost::json::Value& jsv, unsigned_int& v ) { v.value = (uint32_t)jsv.get_uint64();   }
    inline void unpack( const boost::json::Value& jsv, std::string& v )  { v = jsv.get_str();            }

    template<typename T> 
    inline void pack( boost::json::Value& jsv, const T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::pack(jsv,v);
    }

    template<typename T>
    inline void pack( boost::json::Value& jsv, const boost::optional<T>& v ) {
        boost::rpc::json::pack( jsv, *v );
    }

    template<typename T>
    inline void pack( boost::json::Value& jsv, const required<T>& v ) {
        if( !v ) BOOST_THROW_EXCEPTION( boost::rpc::required_field_not_set() );
        boost::rpc::json::pack( jsv, *v );
    }
    template<typename T>
    inline void unpack( const boost::json::Value& jsv, required<T>& v ) {
        v = T();
        boost::rpc::json::unpack( jsv, *v );
    }
    template<typename T>
    inline void unpack( const boost::json::Value& jsv, optional<T>& v ) {
        v = T();
        boost::rpc::json::unpack( jsv, *v );
    }


    template<typename T, typename Alloc, template<typename,typename> class Container>
    inline void pack( boost::json::Value& jsv, const Container<T,Alloc>& value ) {
        jsv = boost::json::Array();
        boost::json::Array& a = jsv.get_array();
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

    template<typename T, typename Alloc, template<typename,typename> class Container>
    inline void unpack( const boost::json::Value& jsv, Container<T,Alloc>& value ) {
        const boost::json::Array& a = jsv.get_array();
        value.resize( a.size() );
        typename Container<T,Alloc>::iterator itr = value.begin();
        typename Container<T,Alloc>::iterator end = value.end();
        uint32_t i = 0;
        while( itr != end ) {
            boost::rpc::json::unpack( a[i], *itr );
            ++itr;
            ++i;
        }
    }

    // support for pair!
    template<typename Key, typename Value>
    inline void pack( boost::json::Value& jsv, const std::pair<Key,Value>& value ) {
        jsv = boost::json::Object();
        jsv.get_obj().push_back( boost::json::Pair( "first", boost::json::Value() ) );
        boost::rpc::json::pack(jsv.get_obj().back().value_, value.first );
        jsv.get_obj().push_back( boost::json::Pair( "second", boost::json::Value() ) );
        boost::rpc::json::pack(jsv.get_obj().back().value_, value.second );
    }
    // support for pair!
    template<typename Key, typename Value>
    void unpack( const boost::json::Value& jsv, std::pair<Key,Value>& value ) {
        const boost::json::Object& obj = jsv.get_obj();
        if( obj.size() < 2 ) {
            BOOST_THROW_EXCEPTION( "Error unpacking pairs" );
        }
        if( obj[0].name_ == "first" )
            boost::rpc::json::unpack( obj[0].value_, value.first );
        if( obj[1].name_ == "second" )
            boost::rpc::json::unpack( obj[1].value_, value.second );
    }


    // support arbitrary key/value containers as an array of pairs
    template<typename Key, typename Value>
    void pack( boost::json::Value& jsv, const std::map<Key,Value>& value ) {
        jsv = boost::json::Array();
        boost::json::Array& a = jsv.get_array();
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
    inline void unpack( const boost::json::Value& jsv, std::map<Key,Value>& value ) {
        const boost::json::Array& a = jsv.get_array();
        value.clear();
        for( uint32_t i = 0; i < a.size(); ++i ) {
            std::pair<Key,Value> p;
            boost::rpc::json::unpack( a[i], p );
            value[p.first] = p.second;
        }
    }

    inline void pack( boost::json::Value& jsv, const std::vector<char>& data ) {
       if( data.size() ) { pack( jsv, base64_encode((unsigned char*)&data.front(),data.size())); } 
    }
    inline void unpack( const boost::json::Value& jsv, std::vector<char>& data ) {
        data.clear();
        std::string d = base64_decode(jsv.get_str());
        data.insert(data.begin(),d.begin(),d.end());
    }


    // pack map<string,T> as a JSON Object
    template<typename Value>
    void pack( boost::json::Value& jsv, const std::map<std::string,Value>& value ) {
        jsv = boost::json::Object();
        boost::json::Object& o = jsv.get_obj();
        o.reserve(value.size());
        typename std::map<std::string,Value>::const_iterator itr = value.begin();
        typename std::map<std::string,Value>::const_iterator end = value.end();
        while( itr != end ) {
            o.push_back( boost::json::Pair(itr->first,boost::json::Value()));
            boost::rpc::json::pack( o.back().value_, itr->second );
            ++itr;
        }
    }
    template<typename Value>
    void unpack( const boost::json::Value& jsv, std::map<std::string,Value>& value ) {
        const boost::json::Object& o = jsv.get_obj();
        value.clear();
        for( uint32_t i = 0; i < o.size(); ++i ) {
            boost::rpc::json::unpack( o[i].value_, value[o[i].name_] );
        }
    }
    template<typename T> 
    inline void unpack( const boost::json::Value& jsv, T& v ) {
        detail::if_fusion_seq< boost::fusion::traits::is_sequence<T>::value >::unpack(jsv,v);
    }

    template<typename T>
    std::ostream& to_json( std::ostream& os, const T& v, uint32_t options = boost::json::remove_trailing_zeros ) {
        boost::json::Value jsv; 
        pack( jsv, v );
        boost::json::write( jsv, os, options );
        return os;
    }
    template<typename T>
    std::ostream& to_jsonf( std::ostream& os, const T& v, 
                            uint32_t options = boost::json::pretty_print | boost::json::remove_trailing_zeros  ) {
        return to_json(os,v,options);
    }
    template<typename T>
    std::string to_json( const T& v, uint32_t options=0 ) {
        std::ostringstream ss;
        to_json( ss, v, options );
        return ss.str();
    }

    template<typename T>
    void from_json( const std::string& j, T& val ) {
        boost::json::Value v;
        boost::json::read( j, v );
        unpack( v, val );
    }

    template<typename T>
    std::string to_jsonf( const T& v, uint32_t options=boost::json::pretty_print ) {
        return to_json(v,options);
    }
    
} } } // namespace boost::rpc::json

#endif // BOOST_RPC_JSON_HPP
