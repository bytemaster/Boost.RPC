#ifndef _BOOST_RPC_JSON_CONNECTION_HPP_
#define _BOOST_RPC_JSON_CONNECTION_HPP_
#include <boost/rpc/json/value_io.hpp>
#include <boost/rpc/error.hpp>
#include <boost/function.hpp>
#include <vector>
#include <boost/cmt/thread.hpp>
#include <boost/signals.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/front.hpp>
#include <boost/fusion/support/deduce.hpp>
#include <boost/mpl/if.hpp>
#include <boost/fusion/include/make_vector.hpp>


namespace boost { namespace rpc { namespace json {
  typedef boost::function<json::value( const json::value& param )> rpc_method;
  struct named_parameters{};

  namespace detail {

     /**
      * If Seq size is 1 and it inherits from named_parameters then Seq will be
      * sent as named parameters.
      */
     template<typename Seq>
     struct has_named_params : 
        public boost::mpl::if_c<
                 (boost::is_base_of<named_parameters, 
                                    typename boost::fusion::traits::deduce<
                                        typename boost::fusion::result_of::front<Seq>::type >::type>::value 
                 && (1 ==  boost::fusion::result_of::size<Seq>::value))
        , boost::true_type, boost::false_type>::type 
     { };

     template<typename Seq, typename Functor,bool NamedParams>
     struct rpc_recv_functor {
       rpc_recv_functor( Functor f )
       :m_func(f){ }

       json::value operator()( const json::value& param ) {
         Seq paramv;
         if( boost::fusion::size(paramv) ) {
            if( !param.is_array() ) {
              BOOST_RPC_THROW( "param value is not an array" );
            }
            json::io::unpack( param, paramv );
         }
         json::value rtn;
         json::io::pack( rtn, m_func(paramv) );
         return rtn;
       }

       Functor m_func;
     };

     /**
      * 
      */
     template<typename Seq, typename Functor>
     struct rpc_recv_functor<Seq,Functor,true> {
       rpc_recv_functor( Functor f )
       :m_func(f){  }

       json::value operator()( const json::value& param ) {
         Seq paramv;
         if( param.is_array() ) {
            json::io::unpack( param, paramv );
         }
         else if( param.is_object() ) {
            json::io::unpack( param, boost::fusion::at_c<0>(paramv) );
         } else {
            BOOST_RPC_THROW( "param value is not an object or array" );
         }
         json::value rtn;
         json::io::pack( rtn, m_func(paramv) );
         return rtn;
       }

       Functor m_func;
     };
     
     // change how params are packed based upon whether or not they are named params
     template<typename Seq>
     void pack_params( json::value& v, const Seq& s, const boost::true_type& is_named ) {
            json::io::pack(v, boost::fusion::at_c<0>(s));
     }
     template<typename Seq>
     void pack_params( json::value& v, const Seq& s, const boost::false_type& is_named ) {
            json::io::pack(v,s);
     }

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


      #include <boost/rpc/json/detail/call_methods.hpp>


      template<typename ParamSeq>
      boost::cmt::future<json::value> call_fused( const std::string& method_name, const ParamSeq& param ) {
        json::value msg;
        msg["method"] = method_name;
        msg["id"]     = next_method_id();
        // TODO: transform functor params...

        if( boost::fusion::size(param ) )
          detail::pack_params( msg["params"], param, typename detail::has_named_params<ParamSeq>::type() );

        typename pending_result_impl<json::value>::ptr pr = boost::make_shared<pending_result_impl<json::value> >(); 

        msg["jsonrpc"] = "2.0";
        send( msg, boost::static_pointer_cast<pending_result>(pr) );
        return pr->prom;
      }


      template<typename R, typename ParamSeq>
      boost::cmt::future<R> call_fused( const std::string& method_name, const ParamSeq& param ) {
        json::value msg;
        msg["method"] = method_name;
        msg["id"]     = next_method_id();
        // TODO: transform functor params...

        if( boost::fusion::size(param ) )
          detail::pack_params( msg["params"], param, typename detail::has_named_params<ParamSeq>::type() );

        typename pending_result_impl<R>::ptr pr = boost::make_shared<pending_result_impl<R> >(); 

        msg["jsonrpc"] = "2.0";
        send( msg, boost::static_pointer_cast<pending_result>(pr) );
        return pr->prom;
      }
      template<typename ParamSeq>
      void notice_fused( const std::string& method_name, const ParamSeq& param ) {
        json::value msg;
        msg["method"] = method_name;
        // TODO: JSON RCP 1.0 sets this to 'null' instead of being empty
        //msg["id"]     = next_method_id();

        // TODO: transform functor params...
         
        // TODO: JSON RPC 1.0 does not allow empty param
        if( boost::fusion::size(param ) )
          detail::pack_params( msg["params"], param, typename detail::has_named_params<ParamSeq>::type() );

        msg["jsonrpc"] = "2.0";
        send( msg );
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
