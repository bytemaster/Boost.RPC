#ifndef _BOOST_RPC_JSON_CONNECTION_HPP_
#define _BOOST_RPC_JSON_CONNECTION_HPP_
#include <boost/rpc/json/value_io.hpp>
#include <boost/function.hpp>
#include <vector>
#include <boost/cmt/thread.hpp>
#include <boost/signals.hpp>

namespace boost { namespace rpc { namespace json {
  typedef boost::function<json::value( const json::value& param )> rpc_method;

  namespace detail {
     template<typename Seq, typename Functor>
     struct rpc_recv_functor {
       rpc_recv_functor( Functor f )
       :m_func(f){}

       json::value operator()( const json::value& param ) {
         Seq paramv;
         json::io::unpack( param, paramv );
         json::value rtn;
         json::io::pack( rtn, m_func(paramv) );
         return rtn;
       }

       Functor m_func;
     };
  }  // namespace detail

  /**
   *  Manages RPC call state including:
   *    - sending invokes, setting return codes, and handling promises
   *    - receiving invokes, calling methods, and sending return codes.
   *
   *  Does not implement communication details which are provided by derived classes
   *  which reimplement send() and call the  protected handler methods.
   */
  class connection : public boost::enable_shared_from_this<connection> {
    public:
      typedef boost::shared_ptr<connection> ptr;
      typedef boost::weak_ptr<connection>   wptr;

      /**
       *  @param t - the thread in which messages will be sent and callbacks invoked
       */
      connection( cmt::thread* t = &cmt::thread::current()  );
      ~connection();

      cmt::thread* get_thread()const;

      void add_method( const std::string& mid, const rpc_method& m );

      template<typename R, typename ParamSeq>
      boost::cmt::future<R> call( const std::string& method_name, const ParamSeq& param ) {
        json::value msg;
        msg["method"] = method_name;
        msg["id"]     = next_method_id();
        json::io::pack(msg["params"],param);

        typename pending_result_impl<R>::ptr pr = boost::make_shared<pending_result_impl<R> >(); 
        send( msg, boost::static_pointer_cast<pending_result>(pr) );
        return pr->prom;
      }

      boost::signal<void()> closed;

    protected:
      virtual void send( const json::value& msg ) = 0;

      void break_promises();
      void handle_notice( const json::value& m );
      void handle_call(   const json::value& m );
      void handle_result( const json::value& m );
      void handle_error(  const json::value& m );

      class pending_result {
        public:
          typedef boost::shared_ptr<pending_result> ptr;
          virtual ~pending_result(){}
          virtual void handle_result( const json::value& data )       = 0;
          virtual void handle_error( const boost::exception_ptr& e  ) = 0;
      };

    private:
      friend class connection_private;

      uint64_t next_method_id();

      void send( const json::value& msg, const connection::pending_result::ptr& pr );


      template<typename R> 
      class pending_result_impl : public pending_result {
        public:
          pending_result_impl():prom(new boost::cmt::promise<R>()){}
          ~pending_result_impl() {
            if( !prom->ready() ) {
              prom->set_exception( boost::copy_exception( boost::cmt::error::broken_promise() ));
            }
          }
          typedef boost::shared_ptr<pending_result_impl> ptr;
          virtual void handle_result( const json::value& data ) {
            R value;
            json::io::unpack( data, value );
            prom->set_value( value );
          }
          virtual void handle_error( const boost::exception_ptr& e  ) {
            prom->set_exception(e);
          }
          typename boost::cmt::promise<R>::ptr prom;
      };
      class connection_private* my;
  };

} } } // boost::rpc::json

#endif
