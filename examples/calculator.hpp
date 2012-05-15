#ifndef _BOOST_REFLECT_CALCULATOR_HPP
#define _BOOST_REFLECT_CALCULATOR_HPP
#include <boost/reflect/mirror_interface.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/reflect/any_ptr.hpp>
#include <boost/cmt/future.hpp>
#include <boost/rpc/json/connection.hpp>

struct Service
{
    std::string name()const;
    int         exit();
};

struct named_param_test : public boost::rpc::json::named_parameters {
  named_param_test(){}
  named_param_test( int _x ):x(_x){}
  named_param_test( int _x, int _y ):x(_x),y(_y){}
  boost::optional<int> x;
  boost::optional<int> y;
};

BOOST_REFLECT( named_param_test, (x)(y) );

struct Calculator : Service
{
    double add( double v );           
    double add2( double v, double v2 );
    double sub( double v );           
    double mult( double v );           
    double div( double v );           

    void   set_callback( int c, const boost::function<std::string(int)>& cb );

    double npt( const named_param_test& p );

    double result()const;
};

BOOST_REFLECT_ANY( Service, (name)(exit) )
BOOST_REFLECT_ANY_DERIVED( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result)(npt)(set_callback) )

#endif // _BOOST_REFLECT_CALCULATOR_HPP
