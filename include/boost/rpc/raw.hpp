#ifndef _BOOST_RPC_RAW_HPP_
#define _BOOST_RPC_RAW_HPP_
#include <boost/rpc/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/rpc/errors.hpp>
#include <boost/rpc/required.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/optional.hpp>
#include <boost/throw_exception.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>

#include <iostream>
#include <boost/rpc/log/log.hpp>

namespace boost { namespace rpc { namespace raw { 

namespace detail {

template<bool IsReflected=true>
struct if_reflected;

template<typename DataStreamType>
class unpack_message_visitor : public boost::reflect::visitor< unpack_message_visitor<DataStreamType> >
{
    public:
    unpack_message_visitor( boost::rpc::datastream<DataStreamType>& os )
    :m_os(os){}
    
    /**
     *  This method handles all fundamental types
     */
    template<typename T, typename Flags>
    inline void unpack_field( T& value, const char* name, Flags key, boost::true_type _is_fundamental ) {
        m_os >> value;
    }

    /**
     *  This method handles the general case where T is a nested type 
     *      (not string,varint,fundamental,container,optional, or required)
     */
    template< typename T, typename Flags>
    void unpack_field( T& value, const char* name, Flags key, boost::false_type _is_not_fundamental ) {
         unpack( value );
    }

    template<typename DST>
    struct visit_sequence
    {
         visit_sequence( datastream<DST>& _ds ):pmv(_ds){}

         mutable unpack_message_visitor<DST>    pmv;

         template<typename T>
         void operator() ( T& v )const
         {
            pmv.unpack_field(v,0,0, typename boost::is_fundamental<T>::type());
         }
    };

    template< typename T>
    void unpack( T& value, boost::false_type _is_not_fundamental, boost::mpl::true_ _is_fusion_sequence) {
        visit_sequence<DataStreamType> unpack_vector(m_os);
        boost::fusion::for_each( value, unpack_vector );
    }


    template< typename T>
    void unpack( T& value, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence) {
         if_reflected<boost::reflect::reflector<T>::is_defined>::unpack( m_os, value );
    }

    void unpack( std::string& value, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence) {
         uint32_t size;// = value.size();
         m_os >> size;
         value.resize(size);
         std::vector<char> tmp(size);
         if( size )
             m_os.read( &tmp.front(), tmp.size() );
         value = std::string(tmp.begin(),tmp.end());
    }

    template< typename T >
    void unpack( T& value, boost::true_type _is_fundamental, boost::mpl::false_ _is_not_fusion_sequence) {
        m_os >> value;
    }
    template< typename T >
    void unpack( T& value ) {
        unpack( value, typename boost::is_fundamental<T>::type(), typename boost::fusion::traits::is_sequence<T>::type() );
    }

    template<typename Flags>
    void unpack_field( signed_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental ) {
         m_os >> value;
    }
    template<typename Flags>
    void unpack_field( unsigned_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental ) {
         m_os >> value;
    }
    template<typename Flags>
    void unpack_field( std::string& value, const char* name, Flags key, boost::false_type _is_not_fundamental ) {
         unpack( value );
    }
    template<typename T, typename Flags>
    void unpack_field( boost::optional<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental ) {
        uint8_t present;
        m_os >> present;
        if( present ) {
            if( !value ) 
                value = T();
            unpack_field( *value, name, key, typename boost::is_fundamental<T>::type() );
        }
    }
    template<typename T, typename Flags>
    void unpack_field( typename boost::rpc::required<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental ) {
        if( !!value )
            unpack_field( *value, name, key, typename boost::is_fundamental<T>::type() ); 
        else    
            BOOST_THROW_EXCEPTION( boost::rpc::required_field_not_set() );
    }

    template<typename Class, typename T, typename Flags>
    void accept_member( Class& c, T (Class::*p), const char* name, Flags key ) {
         unpack_field( c.*p, name, key, typename boost::is_fundamental<T>::type() );
    }
    
    template<typename Field, typename Alloc, template<typename,typename> class Container>
    void unpack( Container<Field,Alloc>& c, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence) {
         uint32_t size;
         m_os >> size;
         if( size )
         {
             c.resize(size);
             typename Container<Field,Alloc >::iterator itr = c.begin();
             typename Container<Field,Alloc >::iterator end = c.end();
             while( itr != end )
             {
                unpack( *itr );//, 0, 0, typename boost::is_fundamental<Field>::type() );
                ++itr;
             }
         }
    }

    template<typename First, typename Second>
    void unpack( std::pair<First,Second>& c, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence) {
         unpack( c.first );
         unpack( c.second );
    }


    template<typename Class, typename Field, typename Alloc, template<typename,typename> class Container, typename Flags>
    void accept_member( Class& c, Container<Field,Alloc> (Class::*p), const char* name, Flags key ) {
        unpack( c.*p );
    }
    private:
        boost::rpc::datastream<DataStreamType>& m_os;
};


template<typename DataStreamType>
class pack_message_visitor : public boost::reflect::visitor< unpack_message_visitor<DataStreamType> >
{
    public:
    pack_message_visitor( boost::rpc::datastream<DataStreamType>& os )
    :m_os(os){}
    
    /**
     *  This method handles all fundamental types
     */
    template<typename T, typename Flags>
    inline void pack_field( const T& value, const char* name, Flags key, boost::true_type _is_fundamental )
    {
        m_os << value;
    }

    /**
     *  This method handles the general case where T is a nested type 
     *      (not string,varint,fundamental,container,optional, or required)
     */
    template< typename T, typename Flags>
    void pack_field( const T& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         pack( value );
    }

    template<typename DST>
    struct visit_sequence
    {
         visit_sequence( datastream<DST>& _ds ):pmv(_ds){}

         mutable pack_message_visitor<DST>    pmv;

         template<typename T>
         void operator() ( const T& v )const
         {
            pmv.pack_field(v,0,0, typename boost::is_fundamental<T>::type());
         }
    };

    template< typename T>
    void pack( const T& value, boost::false_type _is_not_fundamental, boost::mpl::true_ _is_fusion_sequence)
    {
        visit_sequence<DataStreamType> pack_vector(m_os);
        boost::fusion::for_each( value, pack_vector );
    }
    template<typename Field, typename Alloc, template<typename,typename> class Container>
    void pack( const Container<Field,Alloc>& c, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
         m_os << uint32_t( c.size() );
         if( c.size() )
         {
             typename Container<Field,Alloc >::const_iterator itr = c.begin();
             typename Container<Field,Alloc >::const_iterator end = c.end();
             while( itr != end )
             {
                pack( *itr );
                ++itr;
             }
         }
    }

    template<typename First, typename Second>
    void pack( const std::pair<First,Second>& c, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
         pack( c.first );
         pack( c.second );
    }

    template< typename T>
    void pack( const T& value, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
         if_reflected<boost::reflect::reflector<T>::is_defined>::pack( m_os, value );
         //detail::pack_message_visitor<DataStreamType> pack_visitor( m_os );
        // boost::reflect::reflector<T>::visit( value, pack_visitor, -1 );
    }

    void pack( const std::string& value, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
         uint32_t size = value.size();
         m_os << size;
         m_os.write(  value.c_str(), size );
    }

    template< typename T >
    void pack( const T& value, boost::true_type _is_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
        m_os << value;
    }
    template< typename T >
    void pack( const T& value )
    {
        pack( value, typename boost::is_fundamental<T>::type(), typename boost::fusion::traits::is_sequence<T>::type() );
    }

    template<typename Flags>
    void pack_field( const signed_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         m_os << value;
    }
    template<typename Flags>
    void pack_field( const unsigned_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         m_os << value;
    }
    template<typename Flags>
    void pack_field( const std::string& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         pack( value );
    }
    template<typename T, typename Flags>
    void pack_field( const boost::optional<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
        uint8_t present = !!value;
        m_os << present;
        if( present )
            pack_field( *value, name, key, typename boost::is_fundamental<T>::type() );
    }
    template<typename T, typename Flags>
    void pack_field( const typename boost::rpc::required<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
        if( !!value )
            pack_field( *value, name, key, typename boost::is_fundamental<T>::type() ); 
        else    
            BOOST_THROW_EXCEPTION( boost::rpc::required_field_not_set() );
    }

    template<typename Class, typename T, typename Flags>
    void accept_member( const Class& c, T (Class::*p), const char* name, Flags key )
    {
         pack_field( c.*p, name, key, typename boost::is_fundamental<T>::type() );
    }
    
    template<typename Class, typename Field, typename Alloc, template<typename,typename> class Container, typename Flags>
    void accept_member( const Class& c, Container<Field,Alloc> (Class::*p), const char* name, Flags key )
    {
        pack( c.*p );
    }
    private:
        boost::rpc::datastream<DataStreamType>& m_os;
};

    template<bool IsReflected>
    struct if_reflected
    {
        template<typename DataStreamType, typename T>
        inline static void unpack( boost::rpc::datastream<DataStreamType>& ds, T& value )
        {
             detail::unpack_message_visitor<DataStreamType> unpack_visitor( ds );
             boost::reflect::reflector<T>::visit( value, unpack_visitor, -1 );
        }
        template<typename DataStreamType, typename T>
        inline static void pack( boost::rpc::datastream<DataStreamType>& ds, const T& value )
        {
             detail::pack_message_visitor<DataStreamType> pack_visitor( ds );
             boost::reflect::reflector<T>::visit( value, pack_visitor, -1 );
        }
    };
    template<>
    struct if_reflected<false>
    {
        template<typename DataStream, typename T>
        inline static void unpack( DataStream& ds, T& value )
        {
            ds >> value;
        }
        template<typename DataStream, typename T>
        inline static void pack( DataStream& ds, const T& value )
        {
            ds << value;
        }
    };

} // namespace detail 

    template<typename T>
    size_t packsize( const T& v )
    {
        boost::rpc::datastream<size_t>         ps;
        detail::pack_message_visitor<size_t>   size_visitor( ps );
        size_visitor.pack(v);
        return ps.tellp();
    }
    template<typename T>
    void pack( char* loc, size_t loc_size, const T& v )
    {
        boost::rpc::datastream<char*>         ds(loc, loc_size);
        detail::pack_message_visitor<char*>   pack_visitor( ds );
        pack_visitor.pack(v);
    }
    template<typename T, typename DataStreamType>
    void pack( boost::rpc::datastream<DataStreamType>& ds, const T& v )
    {
        detail::pack_message_visitor<DataStreamType>   pack_visitor( ds );
        pack_visitor.pack(v);
    }
    template<typename T>
    void pack( std::vector<char>& msg, const T& v )
    {
        msg.resize(packsize(v));
        pack( &msg.front(), msg.size(), v );
    }
    template<typename T>
    void pack( std::string& msg, const T& v )
    {
        msg.resize(packsize(v));
        pack( msg.c_str(), msg.size(), v );
    }

    template<typename T>
    void unpack( const char* msg, size_t msg_size, T& v )
    {
        boost::rpc::datastream<const char*> ds(msg,msg_size);
        detail::unpack_message_visitor<const char*>   unpack_visitor( ds );
        unpack_visitor.unpack(v);
    }

    template<typename T>
    void unpack( const std::vector<char>& msg, T& v )
    {
        if( msg.size() )
            unpack( &msg.front(), msg.size(), v );
    }


} } }// boost::rpc::raw

#endif //  _BOOST_RPC_DETAIL_RAW_HPP_

