#include <boost/idl/reflect.hpp>
#include <boost/rpc/json.hpp>
#include <boost/rpc/protocol_buffer.hpp>
#include <boost/rpc/datastream.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>

struct Test1 
{
    Test1( uint32_t _a=0, const std::string& _b ="Hello"):a(_a),b(_b){}
    boost::rpc::unsigned_int a;
    boost::rpc::unsigned_int c;
    std::string  b;
};
struct Test1a
{
    Test1a( uint32_t _a=0, const std::string& _b ="Hello1a"):a(_a),c(_b){}
    boost::rpc::unsigned_int a;
    boost::rpc::unsigned_int b;
    std::string  c;
};
struct Test1b
{
    Test1b( uint32_t _a=0, const std::string& _b ="Hello1b"):a(_a),c(_b){}
    boost::rpc::unsigned_int a;
    boost::rpc::unsigned_int b;
    std::string  c;
};

BOOST_IDL_REFLECT(Test1, BOOST_PP_SEQ_NIL, (a)(b)(c) )
BOOST_IDL_REFLECT(Test1a, BOOST_PP_SEQ_NIL, (a)(b)(c) )
BOOST_IDL_REFLECT(Test1b, BOOST_PP_SEQ_NIL, (a)(c)(b) )

struct Test2 
{
    Test2():d(3.1415){}
    boost::rpc::signed_int  x;
    boost::rpc::signed_int  y;
    Test1       t1;
    float      f;
    double      d;
    uint32_t    ui;
    int32_t    i;
    int16_t    i16;
    uint16_t    ui16;
    int8_t    i8;
    uint8_t    ui8;
    std::string a;
    std::string c;
    std::string b;
    std::vector<Test1> vec;
    std::vector<std::string> vec2;
};
BOOST_IDL_REFLECT(Test2, BOOST_PP_SEQ_NIL, (x)(y)(t1)(d)(f)(ui)(i)(ui16)(i16)(ui8)(i8)(a)(c)(b)(vec)(vec2) )

inline std::string hex_dump( const char* data, int len )
{
    std::stringstream ss;
    const char* e = data+len;
    while( e != data )
    {
        ss << std::setfill('0') << std::setw(2) << std::hex << (int16_t(*data) & 0x00ff ) << ' ';
        ++data;
    }
    return ss.str();
}
int main( int argc, char** argv )
{
    Test1 t1_in;
    t1_in.a = 34;
    t1_in.b = "Dan was Here";
    t1_in.c = 34;
    std::cerr << boost::rpc::to_json(t1_in);

    std::vector<char> msg;
    boost::rpc::protocol_buffer::pack( msg, t1_in );

    std::cerr << hex_dump( &msg.front(), msg.size() ) << std::endl;

    Test1 t1_out;
    boost::rpc::protocol_buffer::unpack( msg, t1_out );
    std::cerr << boost::rpc::to_json(t1_out);

try {
    Test1a t1a_out;
    boost::rpc::protocol_buffer::unpack( msg, t1a_out );
    std::cerr << boost::rpc::to_json(t1a_out);
}
catch ( const boost::rpc::protocol_buffer::key_type_mismatch& e )
{
    std::cerr << "Caught Expected Error " << boost::diagnostic_information(e) << std::endl;
}

    Test1b t1b_out;
    boost::rpc::protocol_buffer::unpack( msg, t1b_out );
    std::cerr << boost::rpc::to_json(t1b_out);

    Test2 t2_in;
    t2_in.x = 123;
    t2_in.y = -456;
    t2_in.vec.push_back( Test1(9,"Hi"));
    t2_in.vec.push_back( Test1(8,"Dan"));
    t2_in.vec.push_back( Test1(7,"Win!"));
    t2_in.vec2.push_back( "goodbye" );
    t2_in.vec2.push_back( "world" );
    t2_in.a = "testinga";
    t2_in.b = "testingb";
    t2_in.c = "testingc";

    std::cerr << boost::rpc::to_json(t2_in);
    boost::rpc::protocol_buffer::pack( msg, t2_in );
    std::cerr << hex_dump( &msg.front(), msg.size() ) << std::endl;

    Test2 t2_out;
    boost::rpc::protocol_buffer::unpack( msg, t2_out );
    std::cerr << boost::rpc::to_json(t2_out);

#if 0
    for( int32_t i = -2; i <= 2; ++i )
    {
        v.a.value = i;
        std::stringstream ss;
        pack_message pm(ss);
        visit( v, pm );
    }
        {
        v.a.value = 300;
        std::stringstream ss;
        pack_message pm(ss);
        visit( v, pm );
        std::cerr << 300 << "] " << hex_dump( ss.str().c_str(), ss.str().size() ) << std::endl;
        }
        {
        v.a.value = 150;
        std::stringstream ss;
        pack_message pm(ss);
        visit( v, pm );
        std::cerr << 150 << "] " << hex_dump( ss.str().c_str(), ss.str().size() ) << std::endl;
        }
    {
    std::stringstream ss;
    pack_message pm(ss);
    visit( t2, pm );
    //std::cerr << "t2" << "] " << hex_dump( ss.str().c_str(), ss.str().size() ) << std::endl;

    Test2 t3;
    std::string str = ss.str();
    std::vector<char> vstr(str.begin(),str.end());
    boost::idl::datastream<const char*> ds(&vstr.front(), vstr.size() );
    unpack_message( ds, t3 );
    /*
    std::cerr << "unpack x: "<<t3.x.value<<"\n";
    std::cerr << "unpack y: "<<t3.y.value<<"\n";
    std::cerr << "unpack a: '"<<t3.a<<"'\n";
    std::cerr << "unpack b: '"<<t3.b<<"'\n";
    std::cerr << "unpack c: '"<<t3.c<<"'\n";


    {
        std::stringstream ss;
        pack_message pm(ss);
        visit( t3, pm );
        std::cerr << "t3" << "] " << hex_dump( ss.str().c_str(), ss.str().size() ) << std::endl;
    }
    visitor v;
    visit( t2, v );
    visit( t3, v );
    */
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t2)<<std::endl;
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t3)<<std::endl;
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t3)<<std::endl;
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t3)<<std::endl;
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t3)<<std::endl;
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t3)<<std::endl;
    std::cerr<<"to_json:\n"<<boost::idl::to_json(t3)<<std::endl;
    }
    return 0;
    #endif
}
