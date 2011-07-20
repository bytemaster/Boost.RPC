#ifndef _BOOST_RPC_SERVER_HPP_
#define _BOOST_RPC_SERVER_HPP_

namespace boost { namespace rpc {
    /**
        
        Calculator::ptr calc;
        server<CalculatorConnection> serv( calc );

        CalculatorConnection {
            typedef Calculator InterfaceType
            CalculatorConnection( Calculator::ptr ) {}
        }

        
        server::server


     */

    /**
     *   A new ConnectionType is created for each new connection and is passed the
     *   argument given to the constructor of the server.
     *
     *   This ConnectionType is wrapped by an object containing a
     *   boost::reflect::any<ConnectionType> and a map<name,string(string)> where the
     *   map dispatches calls to the any<InterfaceType> which in turn dispatches 
     *   calls to the ConnectionType.  ConnectionType maintains 'per-connection' data
     *   (such as login-status) and the 'global state' is managed by the value passed
     *   to the new connection.
     */
    template<typename ConnectionType, InterfaceType=ConnectionType>
    class server
    {
       public:
           typedef boost::shared_ptr<server> ptr;

           boost::function< boost::shared_ptr<ConnectionType>() > create_connection;

           void listen( uint16_t port );

           template<typename T>
           rpc_server( T v )
           :reflect::any<InterfaceType>(v)
           {
                start_visit(*this); 
           }

           template<typename T>
           server( const T& con_parm )
           {
                create_connection = boost::bind( &server::create<T>, this, con_param );
           }


           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name ) {
                methods[name] = rpc_functor<typename M::fused_params, M&>(m);
                return true;
           }

           template<typename T> 
           boost::shared_ptr<ConnectionType> create( const T& param ) {
                return new ConnectionType(param);
           }

       private:
           template<typename Seq, typename ResultType>
           friend struct rpc_functor;

           template<typename Seq, typename Functor>
           struct rpc_functor {
               rpc_functor( server& c, const char* name )
               :m_server(c),m_name(name){}

               std::string operator()( const std::string& params ) {
                   Seq paramv;
                   std::istringstream is(params);
                   {
                       boost::archive::binary_iarchive ia(is);
                       deserialize_fusion_vector(ia,paramv);                    
                   }
                   std::ostringstream os;
                   {
                       boost::archive::binary_oarchive oa(os);
                       typename boost::remove_reference<Functor>::type::result_type r = m_func(paramv);
                       oa << r;
                   }
                   return os.str();
               }
               Functor m_func;
           };
    };


} } // namespace boost::rpc

#endif // _BOOST_RPC_SERVER_HPP_

