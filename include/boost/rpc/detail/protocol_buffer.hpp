#ifndef _BOOST_RPC_DETAIL_PROTOCOL_BUFFERS_HPP_
#define _BOOST_RPC_DETAIL_PROTOCOL_BUFFERS_HPP_
#include <boost/rpc/datastream.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/idl/reflect.hpp>
#include <boost/optional.hpp>

namespace boost { namespace rpc { namespace protocol_buffer { namespace detail {

    enum protocol_buf_wire_types
    {
        varint            = 0,
        bit64             = 1,
        length_delimited  = 2,
        bit8              = 3, // custom
        bit16             = 4, // custom
        bit32             = 5
    };

    template<typename T, int fund, uint32_t size>
    struct get_wire_type_impl{ enum wire_type { value = length_delimited  }; };

    template<>
    struct get_wire_type_impl<unsigned_int,0,1>{ enum wire_type { value = varint  }; };
    template<>
    struct get_wire_type_impl<signed_int,0,1>  { enum wire_type { value = varint  }; };

    template<typename T>
    struct get_wire_type_impl<T,1,1>{ enum wire_type { value = bit8  }; };
    template<typename T>
    struct get_wire_type_impl<T,1,2>{ enum wire_type { value = bit16 }; };
    template<typename T>
    struct get_wire_type_impl<T,1,4>{ enum wire_type { value = bit32 }; };
    template<typename T>
    struct get_wire_type_impl<T,1,8>{ enum wire_type { value = bit64 }; };
    template<typename T>
    struct get_wire_type : get_wire_type_impl<T,boost::is_fundamental<T>::value,sizeof(T)>{};


    template<typename T,typename T2>
    bool unpack_message_visitor( boost::rpc::datastream<T2>& is, T& val );

    template<typename DataStreamType>
    class unpack_field_visitor
    {
        boost::rpc::datastream<DataStreamType>& is;
        uint32_t m_field;
        uint32_t m_type;
        public:
        unpack_field_visitor( boost::rpc::datastream<DataStreamType>& _is, uint32_t f = -1, uint32_t t = -1 )
        :is(_is),m_field(f),m_type(t),error(true){}
        bool     error;
        template<typename Class>
        void start( Class, const char* anem ){}

        template<typename Class>
        void end( Class, const char* anem ){}


        friend inline void visit( std::string& str, unpack_field_visitor& v, uint32_t f = -1 )
        { 
            unsigned_int size;
            v.is >> size;
            DataStreamType p = v.is.pos();
            v.is.skip(size.value);
            if( v.is.valid() )
            {
                str = std::string( p, v.is.pos() );
            }
            v.error = !v.is.valid();
        }

        /**
         *  This method handles all fundamental types
         */
        template<typename T, typename Flags>
        void unpack( T& value, const char* name, Flags key, boost::true_type _is_fundamental )
        {
            if( key != m_field || m_type != get_wire_type<T>::value )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }
            is >> value;
            error = !is.valid();
        }
        /**
         *  This method handles the general case where T is a nested type 
         *      (not string,varint,fundamental,container,optional, or required)
         */
        template<typename T, typename Flags>
        void unpack( T& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
        {
            if( key != m_field || m_type != get_wire_type<T>::value )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }
            unsigned_int size;
            is >> size;

            if( is.remaining() >= size.value )
            {
                datastream<DataStreamType> is2( is.pos(), size.value );
                if( !unpack_message_visitor( is2, value ) )
                    return; // error!
            }
            else
            {
                std::cerr << "Error unpacking field '"<<name<<"', size is larger than remaining datastream\n";
                error = true;
            }
            is.skip(size);
            error = !is.valid();
        }
        template<typename Flags>
        void unpack( signed_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
        {
            if( key != m_field || m_type != varint )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }
            is >> value;
            error = false;
        }
        template<typename Flags>
        void unpack( unsigned_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
        {
            if( key != m_field || m_type != varint )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }
            is >> value;
            error = false;
        }
        
        template<typename Flags>
        void unpack( std::string& str, const char* name, Flags key, boost::false_type _is_not_fundamental )
        {
            if( key != m_field || m_type != length_delimited )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }
            unsigned_int size;
            is >> size;
            DataStreamType po = is.pos();

            if( is.remaining() >= size.value )
            {
                str = std::string( po, size.value );
                is.skip(size.value);
                error = false;
                return;
            }
            error = true;
        }

        template<typename T, typename Flags>
        void unpack( boost::optional<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
        {
            if( !value )
                value = T();
            unpack( *value, name, key, boost::is_fundamental<T>::type() );
        }

        /**
         *  This is the method called by the visitor, it gets dispatched to the proper
         *  unpack mehtod.
         */
        template<typename Class, typename T, typename Flags>
        void accept_member( Class& c, T (Class::*p), const char* name, Flags key )
        {
            unpack( c.*p, name, key, typename boost::is_fundamental<T>::type() );
        }
        
        template<typename Class, typename Field, typename Alloc, template<typename,typename> class Container, typename Flags>
        void accept_member( Class& c, Container<Field,Alloc> (Class::*p), const char* name, Flags key )
        {
            if( key != m_field || m_type != length_delimited )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }

            unsigned_int size;
            is >> size;

            if( is.remaining() >= size.value )
            {
                datastream<DataStreamType> is2( is.pos(), size.value );
                (c.*p).resize( (c.*p).size()+1);
                if( !unpack_message_visitor( is2, (c.*p).back() ) )
                {
                    (c.*p).pop_back();
                    return; // error!
                }
            }
            else
            {
                std::cerr << "Error unpacking field '"<<name<<"', size is larger than remaining datastream\n";
                error = true;
            }
            is.skip(size);
            error = !is.valid();
        }

        template<typename Class, typename Alloc, template<typename,typename> class Container, typename Flags>
        void accept_member( Class& c, Container<std::string,Alloc> (Class::*p), const char* name, Flags key )
        {
            if( key != m_field || m_type != length_delimited )
            {
                std::cerr << "Error unpacking field '"<<name<<"', key/type mismatch\n";
                error = true;
                return;
            }

            unsigned_int size;
            is >> size;

            DataStreamType po = is.pos();

            if( is.remaining() >= size.value )
            {
                (c.*p).push_back( std::string( po, size.value ) );
                is.skip(size.value);
                error = false;
                return;
            }
        }

        template<typename Class>
        void not_found( Class c, uint32_t field )
        {
            std::cerr<<"NOT FOUND FIELD "<< field <<std::endl;
        }

    };


    template<typename T, typename T2>
    bool unpack_field( boost::rpc::datastream<T2>& is, T& val, uint32_t key, int type )
    {
        unpack_field_visitor<T2> upfv(is, key, type);
        visit( val, upfv, key );
        return !upfv.error;
    }


    template<typename T,typename T2>
    bool unpack_message_visitor( boost::rpc::datastream<T2>& is, T& val )
    {
        while( is.remaining() )
        {
            // unpack key
            unsigned_int tkey;
            is >> tkey;
            if( !is.valid() )
                return false;

            int type = tkey.value & 0x07;
            uint32_t key  = tkey.value >> 3;
            if( !unpack_field( is, val, key, type ) )   
            {
                std::cerr <<"error unpacking field "<<key<<std::endl;
                return false;
             }
        }
        return true;
    }


template<typename DataStreamType>
class pack_message_visitor
{
    public:
    pack_message_visitor( boost::rpc::datastream<DataStreamType>& os )
    :m_os(os){}

    friend inline void visit( const std::string& str, pack_message_visitor& v )
    { 
        v.m_os.write( str.c_str(), str.size() );
    }
    
    template<typename Class>
    void start( Class, const char* anem ){}

    template<typename Class>
    void end( Class, const char* anem ){}

    template<typename Class, typename T, typename Flags>
    void accept_member( const Class& c, T (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | length_delimited );
    
         std::stringstream ss;
         pack_message_visitor pm(ss);
         visit( c.*p, pm );
    
         unsigned_int s(ss.str().size() );
         m_os << s;
         m_os.write( ss.str().c_str(), s.value );
    }
    
    template<typename Class, typename Flags>
    void accept_member( const Class& c, signed_int (Class::*p), const char* name, Flags key )
    {
         m_os << unsigned_int( (uint32_t(key) << 3) | varint );
         m_os << c.*p;
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, unsigned_int (Class::*p), const char* name, Flags key )
    {
         m_os << unsigned_int( uint32_t(key) << 3 | varint );
         m_os << c.*p;
    }
    
    template<typename Class, typename Flags>
    void accept_member( const Class& c, uint8_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit8 );
         m_os <<c.*p;
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, int8_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit8 );
         m_os <<c.*p;
    }
    
    template<typename Class, typename Flags>
    void accept_member( const Class& c, uint16_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit16 );
         m_os.write( (const char*)&(c.*p), sizeof(c.*p) );
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, int16_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit16 );
         m_os.write( (const char*)&(c.*p), sizeof(c.*p) );
    }
    
    template<typename Class, typename Flags>
    void accept_member( const Class& c, int32_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit32 );
         m_os.write( (const char*)&(c.*p), sizeof(int32_t) );
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, uint32_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit32 );
         m_os.write( (const char*)&(c.*p), sizeof(int32_t) );
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, int64_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit64 );
         m_os.write( (const char*)&(c.*p), sizeof(int64_t) );
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, uint64_t (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit64 );
         m_os.write( (const char*)&(c.*p), sizeof(int64_t) );
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, double (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit64 );
         m_os.write( (const char*)&(c.*p), sizeof(double) );
    }
    template<typename Class, typename Flags>
    void accept_member( const Class& c, float (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | bit32 );
         m_os.write( (const char*)&(c.*p), sizeof(float) );
    }
    
    template<typename Class, typename Flags>
    void accept_member( const Class& c, std::string (Class::*p), const char* name, Flags key )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | length_delimited );
         uint32_t size = (c.*p).size();
         m_os <<unsigned_int( size );
         m_os.write( (c.*p).c_str(), size );
    }
    
    template<typename Class, typename Field, typename Alloc, template<typename,typename> class Container, typename Flags>
    void accept_member( const Class& c, Container<Field,Alloc> (Class::*p), const char* name, Flags key )
    {
         if( (c.*p).size() )
         {
             typename Container<Field,Alloc >::const_iterator itr = (c.*p).begin();
             typename Container<Field,Alloc >::const_iterator end = (c.*p).end();
             while( itr != end )
             {
                 m_os <<unsigned_int( uint32_t(key) << 3 | length_delimited );
    
                 std::stringstream ss;
                 pack_message_visitor pm(ss);
                 visit( *itr, pm );
    
                 unsigned_int s(ss.str().size() );
                 m_os <<s;
                 m_os.write( ss.str().c_str(), s.value );
                 ++itr;
             }
         }
    }
    template<typename Class>
    void not_found( Class c, uint32_t field )
    {
        std::cerr<<"NOT FOUND FIELD "<< field <<std::endl;
    }

    private:
        boost::rpc::datastream<DataStreamType>& m_os;
};

} } } } // boost::rpc::protocol_buffer::detail

#endif //  _BOOST_RPC_DETAIL_PROTOCOL_BUFFERS_HPP_

