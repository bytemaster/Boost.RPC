#ifndef _MACE_REFLECT_CALCULATOR_HPP
#define _MACE_REFLECT_CALCULATOR_HPP
#include <mace/stub/mirror_interface.hpp>
#include <mace/reflect/reflect.hpp>
#include <mace/stub/ptr.hpp>
#include <mace/cmt/future.hpp>
#include <mace/rpc/json/connection.hpp>

struct calc_str {
  calc_str(){};

  std::string operation;
  double left;
  double right;
};

MACE_REFLECT( calc_str, (operation)(left)(right) );

struct Service
{
    std::string name()const;
    int         exit();
};

struct named_param_test : public mace::rpc::json::named_parameters {
  named_param_test(){}
  named_param_test( int _x ):x(_x){}
  named_param_test( int _x, int _y ):x(_x),y(_y){}
  boost::optional<int> x;
  boost::optional<int> y;
};

MACE_REFLECT( named_param_test, (x)(y) );

struct Calculator : Service
{
  //  double match( const std::vector<calc>& batch );
    double add( double v );           
    double add2( double v, double v2 );
    double sub( double v );           
    double mult( double v );           
    double div( const calc_str& v );           

    void   set_callback( int c, const boost::function<std::string(int)>& cb );

    double npt( const named_param_test& p );

    double result()const;
};

MACE_STUB( Service, (name)(exit) )
MACE_STUB_DERIVED( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result)(npt)(set_callback) )

#endif // _MACE_REFLECT_CALCULATOR_HPP
