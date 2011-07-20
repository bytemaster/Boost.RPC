#ifndef _BOOST_REFLECT_CALCULATOR_HPP
#define _BOOST_REFLECT_CALCULATOR_HPP
#include <boost/reflect/mirror_interface.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/reflect/any.hpp>

struct Service
{
    std::string name()const;
    int         exit();
};
struct Calculator : Service
{
    double add( double v );           
    double add2( double v, double v2 );
    double sub( double v );           
    double mult( double v );           
    double div( double v );           
    double result()const;
};

BOOST_REFLECT_ANY( Service, BOOST_PP_SEQ_NIL, (name)(exit) )
BOOST_REFLECT_ANY( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result) )

#endif // _BOOST_REFLECT_CALCULATOR_HPP
