#ifndef _BOOST_REFLECT_CALCULATOR_HPP
#define _BOOST_REFLECT_CALCULATOR_HPP
#include <boost/reflect/mirror_interface.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/reflect/any_ptr.hpp>
#include <boost/cmt/future.hpp>

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
    boost::signal<void(double)>   got_result;
    boost::signal<int(double)>    count;
    boost::signal<float(double)>  get_num;
};

BOOST_REFLECT_ANY( Service, (name)(exit) )
BOOST_REFLECT_ANY_DERIVED( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result)(got_result)(count)(get_num) )

#endif // _BOOST_REFLECT_CALCULATOR_HPP
