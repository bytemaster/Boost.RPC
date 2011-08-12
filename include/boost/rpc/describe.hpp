#ifndef _BOOST_RPC_DESCRIBE_HPP_
#define _BOOST_RPC_DESCRIBE_HPP_
#include <boost/reflect/reflect.hpp>
#include <vector>

namespace boost { namespace rpc { 

    struct description {
        struct field {
            field( const std::string& n, const std::string& t )
            :name(n),type(t){}
            std::string name;
            std::string type;
        };
        std::vector< field > fields;
    };

    namespace detail {
      struct describe_visitor {
        describe_visitor( description& d ):desc(d){}

        template<typename T, typename Class, T(Class::*p)>
        void operator()(const char* name )const {
          std::cerr<<" " << name << std::endl;
          desc.fields.push_back( description::field( name, boost::reflect::get_typename<T>() ) );
        }
        description& desc;
      };
    }

    template<typename T>
    description describe_type() {
        description d; 
        boost::reflect::reflector<T>::visit( detail::describe_visitor(d) );
        return d;
    }

} }

BOOST_REFLECT( boost::rpc::description::field, (name)(type) )
BOOST_REFLECT( boost::rpc::description, (fields) )

#endif
