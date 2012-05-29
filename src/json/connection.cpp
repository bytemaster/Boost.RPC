#include <mace/rpc/json/connection.hpp>
#include <boost/unordered_map.hpp>
#include <mace/rpc/json/value.hpp>
#include <mace/rpc/json/value_io.hpp>
#include <mace/rpc/json/error.hpp>
#include <mace/rpc/error.hpp>


namespace mace { namespace rpc { namespace json {
  typedef std::map<std::string,rpc_method>                                  method_map;

  class connection_private {
    public:
      typedef boost::unordered_map<uint64_t,connection::pending_result::ptr> pending_result_map;

      connection_private( rpc::json::connection& s, mace::cmt::thread* t )
      :self(s), m_thread(t), next_method_id(0),next_callback_id(0) { }

      rpc::json::connection&     self;
      method_map                 methods;
      pending_result_map         pending_results;
      mace::cmt::thread*        m_thread;
      uint64_t                   next_method_id;
      uint64_t                   next_callback_id;

      void break_promises() {
        pending_result_map::iterator itr = pending_results.begin();
        while( itr != pending_results.end() ) {
            itr->second->handle_error( boost::copy_exception(mace::cmt::error::broken_promise()) );
            ++itr;
        }
      }
  };

  connection::connection( mace::cmt::thread* t ) {
    my = new connection_private(*this, t);
  }

  connection::~connection() {
    slog( "%1%", this );
    delete my;
  }

  void connection::break_promises() {
    my->break_promises();
  }


  // called by some thread... 
  void connection::send( const json::value& msg, 
                         const connection::pending_result::ptr& pr  ) {
    if( &mace::cmt::thread::current() != my->m_thread ) {
      my->m_thread->async<void>( 
          boost::bind( &connection::send, this, boost::cref(msg), pr ) ).wait();
      return;
    }
    send(msg); 
    if( pr ) 
        my->pending_results[msg["id"]] = pr;
  }

  std::string connection::add_method( const rpc_method& m ) {
    if( &mace::cmt::thread::current() != my->m_thread ) {
      return my->m_thread->async<std::string>( boost::bind( (std::string (connection::*)(const rpc_method&))&connection::add_method, this, m ) );
    } else {
      std::string mid = "CB" + boost::lexical_cast<std::string>( ++my->next_callback_id );
      my->methods[mid] = m;
      return mid;
    }
  }

  // what thread??? 
  void connection::add_method( const std::string& mid, const rpc_method& m ) {
    if( &mace::cmt::thread::current() != my->m_thread ) {
      my->m_thread->async( boost::bind( &connection::add_method, this, mid, m ) );
    } else {
      my->methods[mid] = m;
    }
  }


  uint64_t connection::next_method_id()             { return ++my->next_method_id; }
  mace::cmt::thread* connection::get_thread()const { return my->m_thread; }

  void connection::handle_call( const json::value& m, json::value& reply ) {
    reply["id"] = m["id"];
    method_map::iterator itr = my->methods.find(m["method"]);
    if( itr != my->methods.end()  ) {
      try {
          if( m.contains( "params" ) )
              reply["result"] = itr->second(*this, m["params"]);
          else
              reply["result"] = itr->second(*this, json::value());
      } catch ( const boost::exception& e ) {
          reply["error"]["code"]    = (int64_t)error_code::server_error;
          reply["error"]["message"] = std::string(boost::diagnostic_information(e));
      } catch ( const std::exception& e ) {
          reply["error"]["code"]    = (int64_t)error_code::server_error;
          reply["error"]["message"] = std::string(boost::diagnostic_information(e));
      }
    } else {
      reply["error"]["code"]    = (int64_t)error_code::method_not_found;
      reply["error"]["message"] = std::string("Invalid Method Name");
    }
    reply["jsonrpc"] = "2.0";
  }

  // uses send() to send reply to call
  void connection::handle_call( const json::value& m ) {
    json::value reply;
    handle_call( m, reply );
    send( reply );
  }
  void connection::handle_notice( const json::value& m ) {
    method_map::iterator itr = my->methods.find(m["method"]);
    if( itr != my->methods.end()  ) {
      try {
          if( m.contains( "params" ) )
              itr->second(*this, m["params"]);
          else
              itr->second(*this, json::value());
      } catch ( const boost::exception& e ) {
        wlog( "%1%", boost::diagnostic_information(e) );
      } catch ( const std::exception& e ) {
        wlog( "%1%", boost::diagnostic_information(e) );
      } catch ( ... ) {
        wlog( "Unhandled exception calling method %1%", (std::string)m["method"] );
      }
    } else {
        wlog( "Invalid method %1%", (std::string)m["method"] );
    }
  }
  void connection::handle_result( const json::value& m ) {
    connection_private::pending_result_map::iterator itr = my->pending_results.find(m["id"]);
    if( itr != my->pending_results.end() ) {
      itr->second->handle_result(*this, m["result"]);
      my->pending_results.erase(itr);
    } else {
      wlog( "Unexpected Result Message" );
    }
  }
  void connection::handle_error( const json::value& m ) {
    if( m.contains("id")) {
      connection_private::pending_result_map::iterator itr = my->pending_results.find(m["id"]);
      if( itr != my->pending_results.end() ) {
         if( m.contains("error") ) {
           std::string emsg = json::to_string(m["error"]);
           itr->second->handle_error( boost::copy_exception( mace::rpc::exception() << err_msg(emsg) ));
         }
         my->pending_results.erase(itr);
        return;
      }
      wlog( "Unexpected error id %1%", (std::string)(m["id"]) );
    } else {
      wlog( "Unexpected Error Message" );
    }
  }
  void connection::send( const json::value& msg ) {
    MACE_RPC_THROW( "mace::rpc::connection::send(json::value) was not overridden" );
  }


} } } // namespace mace::rpc::json
