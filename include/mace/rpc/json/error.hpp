#ifndef _MACE_RPC_JSON_ERRORS_HPP_
#define _MACE_RPC_JSON_ERRORS_HPP_
#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>

namespace mace { namespace rpc { namespace json {
    struct parse_error : public virtual boost::exception, public virtual std::exception {
        const char*  what()const throw()    { return "json_parse_error";     }
        virtual void rethrow()const         { BOOST_THROW_EXCEPTION(*this);        }
    };

    struct error_code {
        enum codes {
          parse_error      = -32700,
          invalid_request  = -32600,
          method_not_found = -32601,
          invalid_params   = -32602,
          internal_error   = -32603,
          server_error     = -32000
        };
    };

} } } // namespace mace::rpc::json

#endif // _MACE_RPC_JSON_ERRORS_HPP_
