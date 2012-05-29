#ifndef  _MACE_RPC_JSON_CLIENT_BASE_HPP
#define  _MACE_RPC_JSON_CLIENT_BASE_HPP
#include <mace/rpc/json/connection.hpp>
#include <mace/rpc/json/named_parameters.hpp>

namespace mace { namespace rpc { namespace json {

class client_base {
  public:
    client_base( const mace::rpc::json::connection::ptr& c )
    :m_con(c){}

    client_base( const std::string& host, uint16_t port );
    client_base(){}

    client_base( const client_base& c )
    :m_con(c.m_con){ slog("this: %2% copy %1%", m_con.get(), this  ); }

    template<typename R, typename ParamSeq>
    inline mace::cmt::future<R> call_fused( const std::string& method, const ParamSeq& param ) {
      //slog( "this: %1%   m_con: %2% ", this, m_con.get() );
      return m_con->call_fused<R,ParamSeq>( method, param );
    }

    template<typename ParamSeq>
    inline void notice_fused( const std::string& method, const ParamSeq& param ) {
      m_con->notice_fused( method, param );
    }

  protected:
      mace::rpc::json::connection::ptr m_con;
};

}}} // namespace mace::rpc::json
#endif //  _MACE_RPC_CLIENT_BASE_HPP
