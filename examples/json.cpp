#include <boost/rpc/json.hpp>
#include <boost/rpc/message.hpp>

struct test {
    int a;
    std::string b;
    std::vector<std::string> data;
    std::map<std::string,std::string> keys;
    std::vector<char> bdata;
};

BOOST_REFLECT( test, BOOST_PP_SEQ_NIL, (a)(b)(data)(keys)(bdata) )

int main( int argc, char** argv ) {
    test t;
    t.a = 5;
    t.b = "hello world";
    t.data.push_back( "Hello" );
    t.data.push_back( "World" );
    t.keys["hello"] = "world";

    t.bdata.insert(t.bdata.begin(), t.b.begin(), t.b.end() );

    std::string jtest = boost::rpc::json::to_json(t);
    std::cerr<<jtest<<std::endl;
    std::cerr<<boost::rpc::json::to_jsonf(t)<<std::endl;

    std::cerr<<"unpack...\n";
    test t2;
    boost::rpc::json::from_json( jtest, t2 );
    std::cerr<<boost::rpc::json::to_jsonf(t2)<<std::endl;

    std::cerr<<boost::rpc::json::to_jsonf(boost::rpc::message())<<std::endl;
    std::cerr<<boost::rpc::json::to_jsonf(t.data)<<std::endl;


    return 0;
}
