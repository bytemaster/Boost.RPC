#ifndef _BOOST_RPC_JSON_HPP_
#define _BOOST_RPC_JSON_HPP_
#include <boost/reflect/reflect.hpp>
#include <boost/rpc/datastream/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/rpc/raw.hpp>
#include <boost/rpc/required.hpp>
#include <boost/rpc/log/log.hpp>
#include <boost/rpc/base64.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace boost { namespace rpc { namespace json {

    template<bool IsReflected=true>
    struct if_reflected;

class json_visitor
{
    public:
    json_visitor(std::ostream& os):depth(0),m_os(os),first(true){}
    int depth;

    template<typename Class>
    void start( Class, const char* anem )
    {
        //m_os << (first ? "" : ",\n");
        if( !first )
            m_os<< ",\n"<<std::setw(4*depth)<<" "<<"{";
        else
            m_os<< "{";
        ++depth;
    }

    template<typename Class>
    void end( Class, const char* anem )
    {
            --depth;
        m_os << std::endl;
        m_os<<std::setw(4*depth)<<" "<<"}";
    }

    template<typename Flags>
    inline void add_field( const signed_int& str, const char* name, Flags f )
    {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : ";
        first = true;
        m_os << (first ? "" : ",\n");
        m_os<<std::setw(4*depth)<<" "<<str.value;
        first = false;
    }
    template<typename Flags>
    inline void add_field( const unsigned_int& str, const char* name, Flags f )
    {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : ";
        first = true;
        m_os << (first ? "" : ",\n");
        m_os<<std::setw(4*depth)<<" "<<str.value;
        first = false;
    }
    template<typename Flags>
    inline void add_field( const std::string& str, const char* name, Flags f )
    { 
        if( name != "" )
        {
            m_os << (first ? "\n" : ",\n");
            m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : ";
            first = true;
            m_os << (first ? "" : ",\n");
        }
        m_os<<std::setw(0*depth)<<" "<<'"'<<str<<'"';
        first = false;
    }
    template <typename T, typename Flags>
    inline void add_field( const boost::optional<T>& o, const char* name, Flags f )
    { 
        if( !!o ) add_field( *o, name, f );
        else
        {
            dlog( "%1% was not set", name );
        }
    }
    template <typename T, typename Flags>
    inline void add_field( const boost::rpc::required<T>& r, const char* name, Flags f )
    { 
        if( !!r ) add_field( *r, name, f );
        else
        {
            dlog( "%1% was not set", name );
        }
    }
    
   template <typename T, typename Flags>
   void add_field( T f, const char* name, Flags  )
   {
        if( name != "" )
        {
            m_os << (first ? "\n" : ",\n");
            m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : ";
        }
        first = true;
        json::if_reflected<boost::reflect::reflector<T>::is_defined>::visit( *this, f );
        first = false;
   }
   template <typename Flags>
   void add_field( char f, const char* name, Flags  )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : ";
        first = true;
        m_os << (first ? "" : ",\n");
        m_os<<std::setw(4*depth)<<" "<<int(f);
        first = false;
   }

    template<typename Class, typename T, typename Flags>
    void accept_member( Class c, T (Class::*p), const char* name, Flags f )
    {
        if_reflected<boost::reflect::reflector<typename boost::remove_const<T>::type>::is_defined>::add_field( *this, c.*p, name, f );
    }

    template<typename Class,  template<typename,typename> class Container, typename Flags>
    void accept_member( Class c, std::vector<char> (Class::*p), const char* name, Flags key )
    {
        std::vector<char> data;
        raw::pack( data, c.*p );
        std::string b64;
        if( data.size() )
            b64 = base64_encode( (unsigned char*)&data.front(), data.size() );
        add_field( b64, name, key );
    }

    template<typename Class, typename Field, typename Alloc, template<typename,typename> class Container, typename Flags>
    void accept_member( Class c, Container<Field,Alloc> (Class::*p), const char* name, Flags key )
    {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : [\n" << std::setw(4*(depth+1))<<" ";
        typename Container<Field,Alloc>::const_iterator itr = (c.*p).begin();
        typename Container<Field,Alloc>::const_iterator end = (c.*p).end();
        ++depth;
        first = true;
        while( itr != end )
        {
            m_os << (first ? "" : ",\n");
            if( !first ) 
            m_os<<std::setw(4*depth)<<" ";// << "{";
            first = true;
            add_field( *itr, (const char*)"", key );
 //           boost::reflect::reflector<Field>::visit( *itr, *this );
 //           m_os<<"\n"<<std::setw(4*depth)<<" "<<"}";
            ++itr;
            first  = false;
        }
        --depth;
        m_os<<"\n"<<std::setw(4*depth)<<" "<<"]";
    }
    template<typename Class, typename Flags>
    void accept_member( Class c, signed_int (Class::*p), const char* name, Flags key )
    {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p).value ;
        first = false;
    }
    
    template<typename Class, typename Flags>
    void accept_member( Class c, unsigned_int (Class::*p), const char* name, Flags key )
    {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p).value ;
        first = false;
    }
   template<typename Class, typename Flags>
   void accept_member( Class c, int8_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << int(c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( const Class& c, uint8_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << int(c.*p) ;
        first = false;
   }
    
   template<typename Class, typename Flags>
   void accept_member( Class c, int16_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( const Class& c, uint16_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( Class c, int32_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( const Class& c, uint32_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( const Class& c, uint64_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( const Class& c, int64_t (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }
   template<typename Class, typename Flags>
   void accept_member( Class c, std::string (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : \"" << (c.*p)<< "\"";
        first = false;
   }
   template<typename Class,typename Flags>
   void accept_member( Class c, double (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }

   template<typename Class,typename Flags>
   void accept_member( Class c, float (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : " << (c.*p) ;
        first = false;
   }

   template<typename Class>
    void not_found( Class c, uint32_t field )
    {
        m_os<<"NOT FOUND FIELD "<< field <<std::endl;
    }
    bool          first;
    std::ostream& m_os;
};

    template<bool IsReflected>
    struct if_reflected
    {
        template<typename Visitor, typename T, typename Flags>
        inline static void add_field( Visitor& v, T value, const char* name, Flags flags )
        {
            v.add_field( value, name, flags );
        }
        template<typename Visitor, typename T>
        inline static void visit( Visitor& jv, const T& v )
        {
            boost::reflect::reflector<T>::visit( v,jv);
        }
    };
    template<>
    struct if_reflected<false>
    {
        template<typename Visitor, typename T, typename Flags>
        inline static void add_field( Visitor& v, const T& value, const char* name, Flags flags )
        {
            std::vector<char> data;
            raw::pack( data, value );
            std::string b64;
            if( data.size() )
                b64 = base64_encode( (unsigned char*)&data.front(), data.size() );
            v.add_field( b64, name, flags );
        }
        template<typename Visitor, typename T>
        inline static void visit( Visitor& jv, T v )
        {
            std::vector<char> data;
            raw::pack( data, v );
            std::string b64;
            if( data.size() )
                b64 = base64_encode( (unsigned char*)&data.front(), data.size() );
            jv.m_os << b64;
        }
    };

} // namepsace json

template<typename T>
std::string to_json( const T& v )
{
    std::ostringstream ss;
    json::json_visitor jv(ss);
    json::if_reflected<boost::reflect::reflector<T>::is_defined>::visit( jv, v );
    return ss.str();
}

} } // namespace boost::rpc
#endif
