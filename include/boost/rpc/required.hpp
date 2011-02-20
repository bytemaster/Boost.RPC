#ifndef __BOOST_RPC_REQUIRED_HPP__
#define __BOOST_RPC_REQUIRED_HPP__
#include <boost/optional.hpp>

namespace boost { namespace rpc { 
    template<typename T>
    struct required : public boost::optional<T>
    {
        typedef boost::optional<T> base;
        required(){}
        required( const T& v )
        :boost::optional<T>(v){}

        using base::operator=;
        using base::operator*;
        using base::operator!;
    };

} } // namespace boost::rpc 
#endif // __BOOST_RPC_REQUIRED_HPP__
