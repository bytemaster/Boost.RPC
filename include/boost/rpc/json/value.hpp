#ifndef _BOOST_RPC_JSON_VALUE_HPP_
#define _BOOST_RPC_JSON_VALUE_HPP_
#include <boost/variant/recursive_variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <stdexcept>
#include <vector>
#include <map>
#include <stdint.h>

namespace boost { namespace rpc { namespace json {
    struct null_t{ 
        null_t(){}
        friend std::ostream& operator<<( std::ostream& os, const null_t&  ){ return os; }
    };
    class object;
    class array;

    typedef boost::variant<  null_t, double, bool, std::string, 
                             boost::recursive_wrapper<object>, 
                             boost::recursive_wrapper<array> > value_variant;

    struct value {
       template<typename T>
       value( const T& v ) :val(v){}
       value( const std::string& v ):val( v ){}
       value( uint64_t v ):val( double(v) ){}
       value( int64_t v ):val( double(v) ){}
       value( char v ):val( double(v) ){}
       value( uint8_t v ):val( double(v) ){}
       value( int16_t v ):val( double(v) ){}
       value( uint16_t v ):val( double(v) ){}
       value():val(null_t()){}

       template<typename T>
       operator const T&()const;
       template<typename T>
       operator T&();

       operator int()const         { return boost::get<double>(val); }
       operator int64_t()const     { return boost::get<double>(val); }
       operator uint64_t()const    { return boost::get<double>(val); }
       operator uint32_t()const    { return boost::get<double>(val); }
       operator uint16_t()const    { return boost::get<double>(val); }
       operator uint8_t()const     { return boost::get<double>(val); }
       operator int16_t()const     { return boost::get<double>(val); }
       operator int8_t()const      { return boost::get<double>(val); }
       operator double()const      { return boost::get<double>(val); }
       operator float()const       { return boost::get<float>(val); }
       operator bool()const        { return boost::get<bool>(val); }
       operator std::string()const { std::stringstream ss; ss << val; return ss.str(); }

       inline value& operator = ( uint64_t v ) { val = (double)v; }
       inline value& operator = ( int64_t v ) { val = (double)v; }
       inline value& operator = ( uint32_t v ) { val = (double)v; }
       inline value& operator = ( int32_t v ) { val = (double)v; }
       inline value& operator = ( uint16_t v ) { val = (double)v; }
       inline value& operator = ( int16_t v ) { val = (double)v; }
       inline value& operator = ( uint8_t v ) { val = (double)v; }
       inline value& operator = ( int8_t v ) { val = (double)v; }

       template<typename T>
       inline value& operator = ( const T& v ) {
          val = v;
          return *this;
       }

       // treat it like an array or object
       value&        operator[]( const std::string& index );
       const value&  operator[]( const std::string& )const;
       value&        operator[]( uint32_t index );
       const value&  operator[]( uint32_t index )const;

       size_t size();
       void   resize( uint32_t size );
       void   clear();


       value_variant val;
    };

    template<typename T> 
    value& operator << ( value& val, const T& v ) {
        return val;
    }

    void read( const std::string& j, value& v );
    void write( const value& v, std::string& j, bool pretty = false );
    std::ostream& write(std::ostream& os, const boost::rpc::json::value& v, bool pretty = false );

    namespace detail {
        struct key_val {
            key_val( const std::string& k = "" ):key(k){}
            std::string key;
            value       val;
        };
    }

    class object {
      public:
        value& operator[]( const std::string& index );
        const value& operator[]( const std::string& index )const;
        friend std::ostream& operator<<( std::ostream& os, const object&  ){ return os; }
        inline void         clear() { keys.clear(); }
        inline void reserve( size_t s ) { keys.reserve(s); }

        std::vector< detail::key_val > keys;
    };

    class array {
      public:
        typedef std::vector<value>::const_iterator const_iterator;

        inline void            clear() { vals.clear(); }
        inline size_t          size()const { return vals.size(); }
        inline value&          operator[]( uint32_t idx )      { return vals[idx]; }
        inline const value&    operator[]( uint32_t idx )const { return vals[idx]; }
        inline value&          back()       { return vals.back();  }
        inline const value&    back()const  { return vals.back();  }
        inline const_iterator  begin()const { return vals.begin(); }
        inline void            push_back( const value& v ) { vals.push_back(v); }
        inline void            resize( size_t s ) { vals.resize(s); }
        inline void reserve( size_t s ) { vals.reserve(s); }

        friend std::ostream& operator<<( std::ostream& os, const array&  ){ return os; }
        std::vector< value > vals;
    };



    namespace detail {
       template<typename Result>
       struct cast_visitor : public boost::static_visitor<Result> {
          template<typename InType>
          Result operator()( const InType& d )const {
            return boost::lexical_cast<Result>(d);
          }
       };

       template<typename T>
       struct index_visitor : public boost::static_visitor<T> {
          index_visitor( int index = 0 ):m_index(index){}
          T operator()( const boost::variant<int,std::string,double,null_t>& )const {
            BOOST_THROW_EXCEPTION( std::out_of_range( "not an object or array" ) );
            typename boost::remove_reference<T>::type* val;
            return *val;
          }
          T operator()( const json::array& d )const {
            return d[m_index];
          }
          T operator()( const json::object& d )const {
            return d[boost::lexical_cast<std::string>(m_index)];
          }
          T operator()( json::array& d )const {
            return d[m_index];
          }
          T operator()( json::object& d )const {
            return d[boost::lexical_cast<std::string>(m_index)];
          }
          int m_index;
       };
       template<typename T>
       struct key_visitor : public boost::static_visitor<T> {
          key_visitor( const std::string& key = 0 ):m_key(key){}

          T operator()( const boost::variant<int,std::string,double,null_t>& )const {
            BOOST_THROW_EXCEPTION( std::out_of_range( "not an object or array" ) );
            typename boost::remove_reference<T>::type* val;
            return *val;
          }
          T operator()( const json::array& d )const {
            return d[boost::lexical_cast<int>(m_key)];
          }
          T operator()( const json::object& d )const {
            return d[m_key];
          }
          T operator()( json::array& d )const {
            return d[boost::lexical_cast<int>(m_key)];
          }
          T operator()( json::object& d )const {
            return d[m_key];
          }
          std::string m_key;
       };
    };

    template<typename T>
    value::operator const T&()const {
       return boost::get<T>(val);
    }
    template<typename T>
    value::operator T&() {
       return boost::get<T>(val);
    }

}}}


#endif
