#ifndef __BOOST_RPC_MESSAGE_HPP_
#define __BOOST_RPC_MESSAGE_HPP_
#include <boost/reflect/reflect.hpp>
#include <boost/rpc/varint.hpp>
#include <boost/rpc/required.hpp>
#include <boost/optional.hpp>

namespace boost { namespace rpc {

using boost::optional;

/**
 * Based upon the JSON-RPC 2.0 Specification
 */
struct error_object : public virtual boost::exception, public virtual std::exception
{
    error_object( int32_t c = 0 )
    :code(c){}

    error_object( int32_t c, const std::string& msg )
    :code(c),message(msg){}

    ~error_object() throw() {}

    required<signed_int>  code;
    optional<std::string> message;
    optional<std::string> data;

    const char*  what()const throw()    { return message ? (*message).c_str() : "error object";  }
    virtual void rethrow()const         { BOOST_THROW_EXCEPTION(*this);                          }
};

/**
 *  This RPC message class is designed to work with the
 *  JSON-RPC 2.0 Specification, but should also
 *  serve well for Google Protocol Buffer spec.
 */
struct message 
{
    message(){}
    message( const std::string& name )
    :method(name){}

    message( const std::string& name, const std::string& param )
    :method(name),params(param){}

    optional<signed_int>     id;        ///< Used to pair request/response
    optional<signed_int>     method_id; ///< Used for effecient calls
    optional<std::string>    method;    ///< Used to call by name
    optional<std::string>    params;    ///< JSON Param Array
    optional<std::string>    result;    ///< Return value
    optional<error_object>   error;     ///< Used in case of errors
};


} } // namespace boost::rpc

BOOST_REFLECT( boost::rpc::error_object, BOOST_PP_SEQ_NIL, (code)(message)(data) )
BOOST_REFLECT( boost::rpc::message, BOOST_PP_SEQ_NIL, (id)(method_id)(method)(params)(result)(error) )

#endif // __BOOST_RPC_MESSAGE_HPP_

