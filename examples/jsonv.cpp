//#include <boost/rpc/json.hpp>
#include <boost/rpc/json/value_io.hpp>
#include <boost/rpc/message.hpp>

struct test {
    int a;
    std::string b;
    std::vector<std::string> data;
    std::vector<std::string> data2;
    std::map<std::string,std::string> keys;
    std::vector<char> bdata;
};

BOOST_REFLECT( test,(a)(b)(data)(data2)(keys)(bdata) )

struct msg {
 int id;
 std::string method;
};
BOOST_REFLECT(msg,(id)(method))

int main( int argc, char** argv ) {

    msg m;
    boost::rpc::json::from_json( " { \"id\":5.0, \"method\":  \"Hello\"}\n ", m );
    std::cout<< boost::rpc::json::to_json(m);

    test t;
    t.a = 5;
    t.b = "hello world";
    t.data.push_back( "Hello" );
    t.data.push_back( "World" );
    t.keys["hello"] = "world";

    t.bdata.insert(t.bdata.begin(), t.b.begin(), t.b.end() );
    std::string jtest = boost::rpc::json::to_json(t);
    std::cerr<<"jtest: " << jtest<<std::endl;
//    std::cerr<<boost::rpc::json::to_jsonf(t)<<std::endl;
    std::cerr<<"unpack benchmark...\n";

    for( uint32_t i = 0; i < 10000; ++i ) {
        test t2;
        boost::rpc::json::from_json( jtest, t2 );
    }
    test t2;
    boost::rpc::json::from_json( jtest, t2 );
    std::cerr<<"t2: "<<boost::rpc::json::to_json(t2)<<std::endl;

    std::cerr<<boost::rpc::json::to_json(boost::rpc::message())<<std::endl;
    std::cerr<<boost::rpc::json::to_json(t.data)<<std::endl;

    boost::rpc::json::from_json( "{}", t2 );

    return 0;
}
