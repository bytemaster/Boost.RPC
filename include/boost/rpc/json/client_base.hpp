#ifndef  _BOOST_RPC_JSON_CLIENT_BASE_HPP
#define  _BOOST_RPC_JSON_CLIENT_BASE_HPP
#include <boost/rpc/json/connection.hpp>
#include <boost/rpc/json/named_parameters.hpp>

namespace boost { namespace rpc { namespace json {

class client_base {
  public:
    client_base( const boost::rpc::json::connection::ptr& c )
    :m_con(c){}

    client_base( const std::string& host, uint16_t port );
    client_base(){}

    template<typename R, typename ParamSeq>
    inline cmt::future<R> call_fused( const std::string& method, const ParamSeq& param ) {
      return m_con->call_fused<R,ParamSeq>( method, param );
    }

    template<typename ParamSeq>
    inline void notice_fused( const std::string& method, const ParamSeq& param ) {
      m_con->notice_fused( method, param );
    }

  protected:
      boost::rpc::json::connection::ptr m_con;
};

}}} // namespace tornet::rpc::json
#endif //  _BOOST_RPC_CLIENT_BASE_HPP
