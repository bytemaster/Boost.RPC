#ifndef __MACE_RPC_JSON_NAMED_PARAMETERS_HPP_
#define __MACE_RPC_JSON_NAMED_PARAMETERS_HPP_
#include <boost/mpl/if.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>

namespace mace { namespace rpc { namespace json {
  struct named_parameters{};
  // namespace detail
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

} } }

#endif // __MACE_RPC_JSON_NAMED_PARAMETERS_HPP_
