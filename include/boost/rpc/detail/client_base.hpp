#ifndef _BOOST_RPC_CLIENT_BASE_HPP_
#define _BOOST_RPC_CLIENT_BASE_HPP_
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/system_error.hpp>

namespace boost { namespace rpc { namespace detail {
    class client_base : public boost::enable_shared_from_this<client_base> {
        public:
            client_base();
            ~client_base();

            std::string invoke( const char* method_name, const std::string& params );
            boost::system::error_code connect_to( const std::string& hostname, const std::string& port, uint64_t timeout_us=-1 );

        private:
            class client_base_private* my;
    };
    
} } } // namespace boost::rpc::detail

#endif
