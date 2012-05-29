#ifndef _JSON_VALUE_HPP_
#define _JSON_VALUE_HPP_
#include <boost/variant/recursive_variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <stdexcept>
#include <vector>
#include <map>
#include <stdint.h>

namespace mace { namespace rpc { namespace json {


    struct null_t{ 
        null_t(){}
        friend std::ostream& operator<<( std::ostream& os, const null_t&  ){ return os; }
    };
    class object;
    class array;

    typedef boost::variant<  null_t, double, bool, std::string, 
                             boost::recursive_wrapper<object>, 
                             boost::recursive_wrapper<array> > value_variant;

    /**
     *  @brief variant type that may hold a string, int, double, array, bool, nil, or object.
     */
    class value {
      public:
       value();
       value( const value& c );
       value( const std::string& v );
       value( const array& a );
       value( const object& a );
       value( double v );
       value( bool v );

       operator int()const;
       operator int64_t()const;
       operator uint64_t()const;
       operator uint32_t()const;
       operator uint16_t()const;
       operator uint8_t()const;
       operator int16_t()const;
       operator int8_t()const;
       operator double()const;
       operator float()const;
       operator bool()const;
       operator std::string()const;
       //operator std::string&();
       operator const json::object&()const;
       operator json::object&();
       operator const json::array&()const;
       operator json::array&();

       value get( const std::string& key )const {
        if( contains(key) ) return (*this)[key];
        return key;
       }
       bool is_array()const;
       bool is_object()const;
       bool is_string()const;
       bool is_null()const;
       bool is_bool()const;
       bool is_real()const;

       bool operator == ( const null_t& t )const;
       inline bool   operator ==( const std::string& v )const { 
          return boost::get<const std::string&>(val) == v;
       }
       bool contains( const std::string& key )const;


       value& operator = ( int64_t v );
       value& operator = ( uint64_t v );
       value& operator = ( int32_t v );
       value& operator = ( uint32_t v );
       value& operator = ( int16_t v );
       value& operator = ( uint16_t v );
       value& operator = ( int8_t v );
       value& operator = ( uint8_t v );
       value& operator = ( double v );
       value& operator = ( bool v );
       value& operator = ( null_t v );
       value& operator = ( const std::string& v );
       value& operator = ( const char* v ) { return *this = std::string(v); }
       value& operator = ( const json::object& v );
       value& operator = ( const json::array& v );

       // treat it like an array or object
       value&        operator[]( const std::string& index );
       const value&  operator[]( const std::string& )const;
       value&        operator[]( const char* index );
       const value&  operator[]( const char* index )const;
       value&        operator[]( uint32_t index );
       const value&  operator[]( uint32_t index )const;

       size_t size();
       void   resize( uint32_t size );
       void   clear();

      // friend std::ostream& operator << ( std::ostream& os, const value& v );
      // friend std::istream& operator >> ( std::istream& os, const value& v );

        value_variant val;
    };

    void from_string( const std::string& j, value& v );
    void to_string( const value& v, std::string& j, bool pretty = false );
    std::ostream& write(std::ostream& os, const json::value& v, bool pretty = false );

    template<typename T>
    std::string to_string( const T& v, bool pretty = false ) {
      value jv; 
      jv <<  v;
      std::string str;
      to_string( jv, str );
      return str;
    }


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
        bool contains( const std::string& key )const;

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
       template<>
       struct cast_visitor<std::string> : public boost::static_visitor<std::string> {
          template<typename InType>
          std::string operator()( const InType& d )const {
            return boost::lexical_cast<std::string>(d);
          }
          std::string operator()( const null_t& d )const {
            return "null";
          }
          std::string operator()( const bool& d )const {
            return d ? "true" : "false";
          }
          std::string operator()( const json::array& d )const {
            return to_string(value(d));
          }
          std::string operator()( const json::object& d )const {
            return to_string(value(d));
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
          key_visitor( const std::string& key, value& v ):m_val(v),m_key(key){}

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
          T operator()( null_t& )const {
            m_val = json::object();
            return m_val[m_key];
          }
          T operator()( json::array& d )const {
            return d[boost::lexical_cast<int>(m_key)];
          }
          T operator()( json::object& d )const {
            return d[m_key];
          }
          value&      m_val;
          std::string m_key;
       };
       template<typename T>
       struct const_key_visitor : public boost::static_visitor<T> {
          const_key_visitor( const std::string& key, const value& v ):m_val(v),m_key(key){}

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
          const value&      m_val;
          std::string m_key;
       };
    }

    value&       operator<<(value& jv, const value& v );
    const value& operator>>(const value& jv, value& v );

    template<typename T>
    value& operator<<(value& jv, const std::vector<T>& v ) {
      json::array& a = jv = json::array();            
      a.resize(v.size());
      for( uint32_t i = 0; i < v.size(); ++i ) 
        a[i] << v[i];
      return jv;
    }

    template<typename T>
    const value& operator>>(const value& jv, std::vector<T>& v ) {
      const json::array& a = jv;
      v.resize(a.size());
      for( uint32_t i = 0; i < v.size(); ++i ) 
        a[i] >> v[i];
      return jv;
    }

} } } // namespace mace::rpc::json


#endif
