#ifndef _BOOST_RPC_PROTOCOL_BUFFERS_HPP_
#define _BOOST_RPC_PROTOCOL_BUFFERS_HPP_
#include <boost/rpc/datastream/datastream.hpp>
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
#include <boost/cmt/log/log.hpp>

namespace boost { namespace rpc { namespace protocol_buffer { 

namespace detail {

    enum protocol_buf_wire_types
    {
        varint            = 0,
        bit64             = 1,
        length_delimited  = 2,
        bit8              = 3, // custom
        bit16             = 4, // custom
        bit32             = 5,
        bit0              = 6  // field is either there or not, size 0
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
    
        /**
         *  This method handles all fundamental types
         */
        template<typename T, typename Flags>
        void unpack( T& value, const char* name, Flags key, boost::true_type _is_fundamental )
        {
            if( key != m_field || m_type != get_wire_type<T>::value )
            {
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
                error = true;
                return;
            }
            is >> value;
            error = !is.valid();
        }
        /**
         *  This method handles all fundamental types
         */
        template<typename T, typename Flags>
        void unpack( T& value, const char* name, Flags key )
        {
            dlog( "key %1%  m_field %2%  m_type %3%", key, m_field, m_type );
            unpack( value, name, key, typename boost::is_fundamental<T>::type() );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::field_size_larger_than_buffer(name) );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
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
            wlog( "unapcking optional %1%", name );
            unpack( *value, name, key,typename boost::is_fundamental<T>::type() ); 
        }
        template<typename T, typename Flags>
        void unpack( boost::rpc::required<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
        {
            if( !value )
                value = T();
            wlog( "unapcking required %1%", name );
            unpack( *value, name, key, typename boost::is_fundamental<T>::type() );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::field_size_larger_than_buffer(name) );
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
                BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::key_type_mismatch(name) );
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
            BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::field_not_found() );
        }

    };


    template<typename T, typename T2, typename Flags>
    bool unpack_field( boost::rpc::datastream<T2>& is, T& val, Flags key, int type )
    {
        unpack_field_visitor<T2> upfv(is, key, type);
        boost::reflect::reflector<T>::visit( val, upfv, key );
        return !upfv.error;
    }
    template<typename T2, typename Flags>
    bool unpack_field( boost::rpc::datastream<T2>& is, std::string& val, Flags key, int type )
    {
        unsigned_int size;
        is >> size;
        T2 p = is.pos();
        is.skip(size.value);
        if( is.valid() )
        {
            val = std::string( p, is.pos() );
        }
        return !is.valid();
    }

    /**
     *  By default, we can simply use the unpack_field() calls which use
     *  reflect::reflect<>, but this class is specialized for fusion sequences.
     */
    template<typename IsSequence>
    struct unpack_message_selector
    {
        template<typename T,typename T2>
        static inline bool unpack_message( boost::rpc::datastream<T2>& is, T& val )
        {
       //     unpack_message_selector<T>::unpack_message( is, val );
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
    };
    /**
     *  Specialization for fusion sequences.
     */
    template<>
    struct unpack_message_selector<boost::integral_constant<bool, true> >
    {
        /**
         *  Run-time access to boost::fusion::vector<> requires special support
         *  to provide "random access" to the fields.  
         *
         * @todo Convert the dynamic key lookup sequence into a binary search
         *       instead of a linear search. Fortunately the number of fields in
         *       the vector should be a relatively small number.
         */
        template<typename Sequence, int32_t Index = 
                                    boost::fusion::result_of::size<Sequence>::type::value>
        struct helper
        {
            template<typename StreamType>
            static inline bool unpack_field( boost::rpc::datastream<StreamType>& is, 
                                             Sequence& seq, uint32_t key, uint32_t type )
            {
                if( (Index -1)== key )
                {
                    unpack_field_visitor<StreamType> ufv( is, key, type );
                    ufv.unpack( boost::fusion::at_c<Index-1>(seq), 0, key ); 
                    return true;
                }
                else
                {
                    helper<Sequence,(Index-1)>::unpack_field( is, seq, key, type );
                }
            }
        };

        template<typename Sequence>
        struct helper<Sequence,0>
        {
            template<typename StreamType>
            static inline bool unpack_field( boost::rpc::datastream<StreamType>& is, 
                                             Sequence& seq, uint32_t key, uint32_t type )
            {
                return false; 
            }
        };

        template<typename Sequence,typename T2>
        static inline bool unpack_message( boost::rpc::datastream<T2>& is, Sequence& val )
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
                if( !helper<Sequence>::unpack_field( is, val, key, type ) )   
                {
                    std::cerr <<"error unpacking field "<<key<<std::endl;
                    return false;
                 }
            }
            return true;
        }
    };


    /**
     *  Iterates over the fields in the message and unpacks each
     *  field as it is found.
     */
    template<typename T,typename T2>
    bool unpack_message_visitor( boost::rpc::datastream<T2>& is, T& val )
    {
        return unpack_message_selector<typename boost::fusion::traits::is_sequence<T>::type>::unpack_message(is,val);
    }


template<typename DataStreamType>
class pack_message_visitor
{
    public:
    pack_message_visitor( boost::rpc::datastream<DataStreamType>& os )
    :m_os(os){}
    
    template<typename Class>
    void start( Class, const char* anem ){}

    template<typename Class>
    void end( Class, const char* anem ){}

    /**
     *  This method handles all fundamental types
     */
    template<typename T, typename Flags>
    void pack_field( const T& value, const char* name, Flags key, boost::true_type _is_fundamental )
    {
        m_os << unsigned_int( uint32_t(key) << 3 | get_wire_type<T>::value );
        m_os << value;
    }

    /**
     *  This method handles the general case where T is a nested type 
     *      (not string,varint,fundamental,container,optional, or required)
     */
    template< typename T, typename Flags>
    void pack_field( const T& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | length_delimited );
    
         unsigned_int s(boost::rpc::protocol_buffer::packsize(value));
         m_os << s;
         pack( value );
    }

    template<typename DST>
    struct visit_sequence
    {
         visit_sequence( datastream<DST>& _ds ):id(0),pmv(_ds){}

         mutable uint32_t                     id;
         mutable pack_message_visitor<DST>    pmv;

         template<typename T>
         void operator() ( const T& v )const
         {
            pmv.pack_field(v,0,id, typename boost::is_fundamental<T>::type());
            ++id;
         }
    };

    template< typename T>
    void pack( const T& value, boost::false_type _is_not_fundamental, boost::mpl::true_ _is_fusion_sequence)
    {
        visit_sequence<DataStreamType> pack_vector(m_os);
        boost::fusion::for_each( value, pack_vector );
    }
    template< typename T>
    void pack( const T& value, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
         detail::pack_message_visitor<DataStreamType> pack_visitor( m_os );
         boost::reflect::reflector<T>::visit( value, pack_visitor, -1 );
    }

    void pack( const std::string& value, boost::false_type _is_not_fundamental, boost::mpl::false_ _is_not_fusion_sequence)
    {
         uint32_t size = value.size();
         m_os <<unsigned_int( size );
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
         m_os << unsigned_int( uint32_t(key) << 3 | varint );
         m_os << value;
    }
    template<typename Flags>
    void pack_field( const unsigned_int& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         m_os << unsigned_int( uint32_t(key) << 3 | varint );
         m_os << value;
    }
    template<typename Flags>
    void pack_field( const std::string& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
         m_os <<unsigned_int( uint32_t(key) << 3 | length_delimited );
         pack( value );
    }
    template<typename T, typename Flags>
    void pack_field( const boost::optional<T>& value, const char* name, Flags key, boost::false_type _is_not_fundamental )
    {
        if( !!value )
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
         if( (c.*p).size() )
         {
             typename Container<Field,Alloc >::const_iterator itr = (c.*p).begin();
             typename Container<Field,Alloc >::const_iterator end = (c.*p).end();
             while( itr != end )
             {
                pack_field( *itr, name, key, typename boost::is_fundamental<Field>::type() );
                ++itr;
             }
         }
    }
    template<typename Class>
    void not_found( Class c, uint32_t field )
    {
        BOOST_THROW_EXCEPTION( boost::rpc::protocol_buffer::field_not_found() );
    }

    private:
        boost::rpc::datastream<DataStreamType>& m_os;
};

} // namespace detail 

    template<typename T>
    size_t packsize( const T& v )
    {
        boost::rpc::datastream<size_t>         ps;
        detail::pack_message_visitor<size_t>   size_visitor( ps );
        size_visitor.pack(v);
     //   boost::reflect::reflector<T>::visit( const_cast<T&>(v), size_visitor, -1 );
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
        elog( "unpack '%1%'", boost::reflect::get_typeinfo<T>::name() );
        boost::rpc::datastream<const char*> ds(msg,msg_size);
        detail::unpack_message_visitor( ds, v );
   //     detail::unpack_field_visitor<const char*> visitor(ds,0,detail::get_wire_type<T>::value );
   //     visitor.unpack( v, 0, 0 );

    }

    template<typename T>
    void unpack( const std::vector<char>& msg, T& v )
    {
        if( msg.size() )
            unpack( &msg.front(), msg.size(), v );
    }


} } }// boost::rpc::protocol_buffer

#endif //  _BOOST_RPC_DETAIL_PROTOCOL_BUFFERS_HPP_

