#ifndef _BOOST_RPC_JSON_CONNECTION_HPP_
#define _BOOST_RPC_JSON_CONNECTION_HPP_
#include <boost/cmt/future.hpp>
#include <boost/rpc/json.hpp>
#include <boost/cmt/retainable.hpp>
#include <boost/function.hpp>
#include <boost/signals.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/functional/generation/make_unfused.hpp>
#include <boost/bind.hpp>
#include <boost/rpc/message.hpp>

namespace boost { namespace rpc { namespace json {
  /**
   *  @brief JSON-RPC connection interface.
   *
   *  This class serves as the base class for various transports such as TCP, UDP, etc and
   *  provides a common interface as well as default implementation of common state-handling
   *  logic such as waiting for return values and managing signal connections.
   */
  class connection : public boost::cmt::retainable, boost::noncopyable {
    public:
      typedef boost::cmt::retainable_ptr<connection>              ptr;
      typedef boost::function<void(const boost::json::Value&, boost::json::Value&)> handler;
  
      connection();
      virtual ~connection();

      void add_signal_connection( const std::string& name, 
                                  const boost::signals::connection& c );
      void add_method_handler( const std::string& name, const handler& h ); 
      void invoke( boost::json::Value& msg, boost::json::Value& rtn_msg, 
                   const boost::chrono::microseconds& timeout_us = boost::chrono::microseconds::max() );

      virtual void send( const boost::json::Value& v )=0;
      virtual void start()=0;
      virtual bool is_connected()const = 0;

      boost::signal<void()> disconnected;
    protected:
      void on_receive( const boost::json::Value& v );

    private:
      class connection_private* my;
  };

  namespace detail {

    template<typename Seq,typename ResultType>
    struct rpc_send_functor {
      typedef ResultType result_type;

      rpc_send_functor( connection& c, const char* name )
      :m_con(c),m_msg(boost::json::Object()) 
      {
        boost::json::Object&  m_obj = m_msg.get_obj(); // obj stored in m_msg
        m_obj.push_back( boost::json::Pair( "id", 0 ) );
        m_obj.push_back( boost::json::Pair( "method", std::string(name) ) );
        m_obj.push_back( boost::json::Pair( "params", boost::json::Array() ) );
      }

      ResultType operator()( const Seq& params ) const  {
        boost::json::Object&  m_obj = m_msg.get_obj(); // obj stored in m_msg
        pack( m_obj.back().value_, params );
        boost::json::Value  rtn_msg;
        m_con.invoke( m_msg, rtn_msg );
        ResultType  ret_val;
        if(rtn_msg.contains("error") ) {
          error_object e;
          unpack( rtn_msg["error"], e );
          BOOST_THROW_EXCEPTION( e );
        }
        if( rtn_msg.contains( "result" ) )  {
          unpack( rtn_msg["result"], ret_val );
          return ret_val;
        }
        error_object e;
        e.message = "invalid json RPC message, missing result or error";
        BOOST_THROW_EXCEPTION( e );
      }
      connection&          m_con;
      mutable boost::json::Value    m_msg;
    }; // rpc_send_functor
    template<typename Seq,typename ResultType>
    struct rpc_send_functor<Seq, boost::cmt::future<ResultType> > {
      typedef ResultType result_type;

      rpc_send_functor( connection& c, const char* name )
      :m_con(c),m_msg(boost::json::Object()) 
      {
        boost::json::Object&  m_obj = m_msg.get_obj(); // obj stored in m_msg
        m_obj.push_back( boost::json::Pair( "id", 0 ) );
        m_obj.push_back( boost::json::Pair( "method", std::string(name) ) );
        m_obj.push_back( boost::json::Pair( "params", boost::json::Array() ) );
      }

      ResultType operator()( const Seq& params ) const  {
        boost::json::Object&  m_obj = m_msg.get_obj(); // obj stored in m_msg
        pack( m_obj.back().value_, params );
        boost::json::Value  rtn_msg;
        m_con.invoke( m_msg, rtn_msg );
        ResultType  ret_val;
        if(rtn_msg.contains("error") ) {
          error_object e;
          unpack( rtn_msg["error"], e );
          BOOST_THROW_EXCEPTION( e );
        }
        if( rtn_msg.contains( "result" ) )  {
          unpack( rtn_msg["result"], ret_val );
          return ret_val;
        }
        error_object e;
        e.message = "invalid json RPC message, missing result or error";
        BOOST_THROW_EXCEPTION( e );
      }
      connection&          m_con;
      mutable boost::json::Value    m_msg;
    }; // rpc_send_functor



    template<typename Seq, typename Functor, bool is_signal = false>
    struct rpc_recv_functor {
      rpc_recv_functor( Functor f, connection&, const char* )
      :m_func(f){}
      void operator()( const boost::json::Value& params, boost::json::Value& rtn ) {
        Seq paramv;
        unpack( params, paramv );
        pack( rtn, m_func(paramv) );
      }
      Functor m_func;
    };

    /*
     *  Blocks a signal if it is currently unblocked and 
     *  unblocks it when it goes out of scope if it was blocked
     *  when constructed. 
     */
    struct scoped_block_signal {
      scoped_block_signal( boost::signals::connection& _c )
      :c(_c),unblock(false){ 
        if( c != boost::signals::connection() && !c.blocked() )  {
          unblock = true;
          c.block();
        }
      }
      ~scoped_block_signal() { 
        if( unblock && c != boost::signals::connection() ) 
            c.unblock(); 
      }
      private:
        bool                        unblock;
        boost::signals::connection& c; 
    };
    template<typename Seq, typename Functor>
    struct rpc_recv_functor<Seq,Functor,true> {
      typedef typename boost::remove_reference<Functor>::type functor_type;
      rpc_recv_functor( Functor f, connection& c, const char* name )
      :m_name(name),m_con(c),m_func(f){
        wlog( "rpc_recv_functor %1% %2%", this, name );
        m_sig_con = m_func.connect( rpc_send_functor<Seq,
                                    typename functor_type::result_type>( m_con, m_name ) );
        m_sig_con.block(); 
        c.add_signal_connection( name, m_sig_con );
      }


      void operator()( const boost::json::Value& params, boost::json::Value& rtn ) {
        wlog( "rpc_recv_functor %1%", this );
        scoped_block_signal block_reverse(m_sig_con);
        Seq paramv;
        unpack( params, paramv );
        pack( rtn, m_func(paramv) );
      }

      boost::signals::connection m_sig_con;
      const char*                m_name;
      connection&                m_con;
      Functor                    m_func;
    };

    template<typename Seq, bool Sig, typename Functor>
    inline rpc_recv_functor<Seq,Functor,Sig> make_rpc_recv_functor( Functor f, connection& c, const char* n ) {
        return rpc_recv_functor<Seq,Functor,Sig>(f,c,n);
    }
    using boost::reflect::void_t;
    typedef boost::fusion::vector<std::string,int>         connect_signal_params;
    typedef rpc_send_functor<connect_signal_params,void_t> rpc_connect_signal_base;

    struct rpc_connect_delegate : public rpc_connect_signal_base {
      public:
        rpc_connect_delegate( connection& c, const char* name )
        :rpc_connect_signal_base(c,"rpc_connect_signal"),m_name(name){}

        void operator()( int count )const {
          connect_signal_params v(m_name,count);
          rpc_connect_signal_base::operator()(v);
        }
      private:
        std::string          m_name;
    };

    template<bool IsSignal = false> 
    struct if_signal {
      template<typename M>
      static void set_delegate( connection& c, M& m, const char* name ){
        m = detail::rpc_send_functor<typename M::fused_params, 
                                     typename M::result_type>(c, name);
      }
    };

    template<> 
    struct if_signal<true> {
      template<typename M>
      static void set_delegate( connection& c, M& m, const char* name ){
        m = detail::rpc_send_functor<typename M::fused_params, 
                                     typename M::result_type>(c, name);

        // when client connects/disconnects local signals, notify server of change
        m.set_connect_delegate(rpc_connect_delegate(c,name));

        // when server calls the client, emit locally, but do not 'echo'
        // back to the server
        c.add_method_handler( name, 
             rpc_recv_functor<typename M::fused_params, 
                boost::function<typename M::result_type(typename M::fused_params)> >(
                boost::bind(&M::emit, &m, _1), c, name) );
      }
    };
  
  } // namespace detail

} } } // boost::rpc::json

#endif// _BOOST_RPC_JSON_CONNECTION_HPP_
