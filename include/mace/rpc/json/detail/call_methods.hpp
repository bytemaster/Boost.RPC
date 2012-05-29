#ifndef BOOST_PP_IS_ITERATING

#        ifndef BOOST_CALL_IMPL_SIZE
#           define BOOST_CALL_IMPL_SIZE 8
#        endif

    #define PARAM_NAME(z,n,type)         BOOST_PP_CAT(a,n)
    #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
    #define PARAM_TYPE(z,n,type)   BOOST_PP_CAT(A,n)
    #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_CALL_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <mace/rpc/json/detail/call_methods.hpp>
#       include BOOST_PP_ITERATE()

#undef PARAM_NAME
#undef PARAM_ARG
#undef PARAM_TYPE_NAME
#undef PARAM_TYPE


#else // BOOST_PP_IS_ITERATING

    #define n BOOST_PP_ITERATION()
    #define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
    #define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
    #define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename TYPE_N
    #define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N


    #if n > 0
      template<PARAM_TYPE_NAMES>
      mace::cmt::future<rpc::json::value> call( const std::string& method_name, PARAM_ARGS ) {
        return call_fused<rpc::json::value>( method_name, boost::fusion::make_vector( PARAM_NAMES ) );
      }
      template<typename R,PARAM_TYPE_NAMES>
      mace::cmt::future<R> call( const std::string& method_name, PARAM_ARGS ) {
        return call_fused<R>( method_name, boost::fusion::make_vector( PARAM_NAMES ) );
      }
   #else
      mace::cmt::future<rpc::json::value> call( const std::string& method_name ) {
        return call_fused<rpc::json::value>( method_name, boost::fusion::make_vector( ) );
      }
      template<typename R>
      mace::cmt::future<R> call( const std::string& method_name ) {
        return call_fused<R>( method_name, boost::fusion::make_vector( ) );
      }
   #endif


#undef PARAM_TYPE_NAMES 
#undef PARAM_NAMES         
#undef PARAM_ARGS        
#undef PARAM_TYPES

#endif
