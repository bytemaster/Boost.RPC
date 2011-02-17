#ifndef _BOOST_IDL_JSON_HPP_
#define _BOOST_IDL_JSON_HPP_
#include <boost/idl/reflect.hpp>
#include <boost/rpc/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace boost { namespace rpc {

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

    friend inline void visit( const signed_int& str, json_visitor& v, uint32_t f = -1 )
    {
        v.m_os << (v.first ? "" : ",\n");
        v.m_os<<std::setw(4*v.depth)<<" "<<str.value;
        v.first = false;
    }
    friend inline void visit( const unsigned_int& str, json_visitor& v, uint32_t f = -1 )
    {
        v.m_os << (v.first ? "" : ",\n");
        v.m_os<<std::setw(4*v.depth)<<" "<<str.value;
        v.first = false;
    }
    friend inline void visit( const std::string& str, json_visitor& v, uint32_t f = -1 )
    { 
        v.m_os << (v.first ? "" : ",\n");
        v.m_os<<std::setw(0*v.depth)<<" "<<'"'<<str<<'"';
        v.first = false;
    }

   template<typename Class, typename T, typename Flags>
   void accept_member( Class c, T (Class::*p), const char* name, Flags f )
   {
        m_os << (first ? "\n" : ",\n");
        m_os<<std::setw(4*depth)<<" "<< '"'<<name<< "\" : ";
        first = true;
//        ++depth;
        visit( c.*p, *this );
//        --depth;
//        m_os<<"\n"<<std::setw(4*depth)<<" "<<"}";
        first = false;
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
            visit( *itr, *this );
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

template<typename T>
std::string to_json( const T& v )
{
    std::ostringstream ss;
    json_visitor jv(ss);
    visit( v,jv);
    return ss.str();
}

} } // namespace boost::rpc
#endif
