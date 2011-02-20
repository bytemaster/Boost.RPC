#ifndef _BOOST_RPC_CLIENT_HPP_
#define _BOOST_RPC_CLIENT_HPP_
#include <boost/utility/result_of.hpp>
#include <boost/rpc/connection.hpp>
#include <boost/optional.hpp>

namespace boost { namespace rpc {

using boost::optional;

/**
 *  @class boost::rpc::client
 *  @brief Provides a generic RPC connection.
 *
 *
 *
 */
template<typename Protocol>
class client
{
    public:
        client( const typename connection<Protocol>::ptr& con )
        :m_con(con)
        {
            con->_closed           = boost::bind( &client::handle_closed, this );
            con->_received_message = boost::bind( &client::handle_message, this, _1 );
        }

        template<typename Result, typename Functor>
        struct closure
        {
            closure( Functor f):m_fun(f){}
            void operator()( const optional<std::string>& result, int err )
            {
               // typedef typename boost::result_of<Functor>::type rt;
              //  rt r;
              //  typename boost::result_of<int()>::type r;
                Result r;

                //typename boost::remove_reference<typename boost::remove_const<typename Functor::result_type>::type>::type r;
                try {
                    Protocol::unpack( result->c_str(), result->size(), r );
                    m_fun( r, err ); 
                } 
                catch ( ... )
                {
                    m_fun( r, -1 ); 
                }
            }
            Functor m_fun; 
        };

        void notify( const std::string& name )
        {
            m_con->send_message(message(name));
        }

        template<typename Params>
        void notify( const std::string& name, const Params& params )
        {
            message msg( name );
            size_t param_size = Protocol::packsize( params );
            if( param_size )
            {
                msg.params = std::string();
                msg.params->resize(param_size);
                Protocol::pack( msg.params->c_str(), param_size, params );
            }
            m_con->send_message( msg );
        }

        template<typename Result, typename Closure>
        void call( const std::string& name, Closure c )
        {
            message msg( name );
            msg.id                    = next_req_id;
            m_closures[next_req_id++] = closure<Result, Closure>(c);
            m_con->send_message( msg );
        }

        /**
         *  Use boost::fusion::make_vector(arg1,arg2,arg3,...)
         */
        template<typename Result, typename Closure, typename Params>
        void call( const std::string& name, Closure c, const Params& params )
        {
            message msg( name );
            size_t param_size = Protocol::packsize( params );
            if( param_size )
            {
                msg.params = std::string();
                msg.params->resize(param_size);
                Protocol::pack( (char*)msg.params->c_str(), param_size, params );
            }
            msg.id                = next_req_id;
            m_closures[next_req_id++] = closure<Result, Closure>(c);
            m_con->send_message( msg );
        }

    private:
        void handle_closed()
        {
            closure_map::iterator itr = m_closures.begin();
            while( itr != m_closures.end() )
            {
                itr->second( boost::optional<std::string>(), -2 );
                ++itr;
            }
            m_closures.clear();
        }
        void handle_message( const message& m )
        {
            if( !!m.id )
            {
                closure_map::iterator itr = m_closures.find( *m.id );
                if( itr != m_closures.end() )
                {
                    itr->second( m.result, 0 ); 
                    m_closures.erase(itr);
                }
            }
        }
        typename connection<Protocol>::ptr m_con;        

        uint16_t next_req_id;
        typedef std::map<uint16_t, boost::function<void(const boost::optional<std::string>&, int)> > closure_map;
        closure_map m_closures;
};



} } // namespace boost::rpc

#endif // BOOST_RPC_CLIENT_HPP_

