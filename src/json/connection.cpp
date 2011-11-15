#include <boost/rpc/json/connection.hpp>
#include <map>
#include <boost/cmt/thread.hpp>
#include <boost/bind.hpp>
#include <boost/fusion/functional/generation/make_fused_function_object.hpp>

namespace boost { namespace rpc { namespace json {

  using boost::reflect::void_t;

  typedef boost::function<void(const boost::rpc::json::value&, boost::rpc::json::value&)> json_method;
  typedef std::map<std::string,json_method >                        method_map;
  typedef std::map<std::string,boost::signals::connection >         signal_map;
  typedef std::map<int,boost::cmt::promise<boost::rpc::json::value>* >   pending_req_map;
  typedef boost::fusion::vector<std::string,int>                    connect_signal_params;
  typedef boost::function<void_t(const connect_signal_params&)>     connect_signal_functor;


  class connection_private {
    public:
     method_map       methods;
     signal_map       signals;
     pending_req_map  pending_req;
     int              next_id;

     void_t rpc_connect_signal( const connect_signal_params& param) {
          std::string sig   = boost::fusion::at_c<0>(param);
          int         count = boost::fusion::at_c<1>(param);

          wlog( "connection_private: %1%  sig %2%  ", this, sig );
          signal_map::iterator itr = signals.find(sig);
          if( itr != signals.end() ) {
              itr->second.block( count == 0 );
              return boost::reflect::void_t();
          }
          error_object e;
          e.message = "Unknown signal '"+sig+"'";
         // BOOST_THROW_EXCEPTION( e );
          return boost::reflect::void_t();
     }
  };

    /*
  struct my_functor {
    my_functor(connection_private* cp):conp(cp){}
    connection_private* conp;
    void_t operator()( const connect_signal_params& p )const  {
        return conp->rpc_connect_signal(p);
    }
  };
  */
  template<typename T>
  void test( T t ) {
    boost::function<void_t(const connect_signal_params&)> f = t;
    f( connect_signal_params( "helloworld", 1 ) );
    boost::function<void_t(const connect_signal_params&)> f2;
    f2 = f;
    f2( connect_signal_params( "helloworld", 2 ) );
  }
  connection::connection() {
    my = new connection_private();
    wlog( "this: %1% my %2%", this, my );

    /*
    add_method_handler( "rpc_connect_signal", 
        detail::rpc_recv_functor<connect_signal_params,my_functor,false> (
                my_functor(my), 
                *this, "rpc_connect_signal" ) );
                */
    add_method_handler( "rpc_connect_signal",
        detail::rpc_recv_functor<connect_signal_params,connect_signal_functor,false> (
                boost::bind( &connection_private::rpc_connect_signal, my, _1 ), 
                *this, "rpc_connect_signal" ) ); 


    /*
    add_method_handler( "rpc_connect_signal", 
        detail::make_rpc_recv_functor<connect_signal_params,false>( 
                boost::bind( &connection_private::rpc_connect_signal, my, _1 ), *this, "rpc_connect_signal" ) );
                */


//    test( boost::bind( &connection_private::rpc_connect_signal, my, _1 ) );
//    f( connect_signal_params( "helloworld", 1 ) );

                
  }
  connection::~connection() { 
    wlog( "delete %1%", my );
    delete my; 
  }

  void connection::add_signal_connection( const std::string& name, 
                                          const boost::signals::connection& c ) {
    std::cerr<<this<<" my: "<<my<<" add signal con: " << name << std::endl;
    my->signals[name] = c;
  }
  void connection::add_method_handler( const std::string& name, const json_method& h ) {
    my->methods[name] = h;
  }

  void connection::invoke( boost::rpc::json::value& msg, boost::rpc::json::value& rtn_msg, const boost::chrono::microseconds& timeout_us ) {
    int id = ++my->next_id;
    msg["id"] = id;

    boost::cmt::stack_retainable<boost::cmt::promise<boost::rpc::json::value> > p;
    my->pending_req[id] = &p;

    send( msg );
    try {
        rtn_msg = p.wait(timeout_us);
    } catch ( const boost::exception& e ) {
        my->pending_req.erase(my->pending_req.find(id));
        throw;
    }
  }
  
  void connection::on_receive( const boost::rpc::json::value& v ) {
     if( v.contains( "method" ) ) {
       std::string       name  = v["method"];
       boost::rpc::json::value result;
       result["id"]     = v["id"];
       try {
         method_map::const_iterator itr = my->methods.find(name);
         if( itr != my->methods.end() ) {
           itr->second( v["params"], result["result"] );
         } else {
           boost::rpc::error_object eobj(0);
           eobj.message = "Unknown method '"+name+"'";
           pack( result["error"], eobj );
         }
       } catch ( const std::exception& e ) {
         boost::rpc::error_object eobj(0);
         eobj.message = std::string(e.what());//boost::diagnostic_information(e);
         pack( result["error"], eobj );
       } catch ( const boost::exception& e ) {
         boost::rpc::error_object eobj(0);
         eobj.message = boost::diagnostic_information(e);
         pack( result["error"], eobj );
       }
       send(result);
     } else {
       if( v.contains( "id" ) ) {
         int id = v["id"];
         pending_req_map::iterator itr = my->pending_req.find(id);
         if( itr != my->pending_req.end() ) {
           itr->second->set_value(v);
           my->pending_req.erase(itr);
         } else {
           elog( "unhandled message: '%1%'", boost::rpc::json::to_string(v) );
         }
       } else {
         elog( "message has no 'id' field: '%1%'", boost::rpc::json::to_string(v) );
       }
    }
  }

} } } // boost::rpc::json
