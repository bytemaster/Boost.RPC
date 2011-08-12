#include <boost/rpc/raw.hpp>
#include <boost/rpc/describe.hpp>
#include <boost/rpc/json/value_io.hpp>
#include <boost/chrono.hpp>
#include <boost/rpc/datastream.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

struct substruct {
    double hi;
    double low;
};

struct test {
    int a;
    std::string b;
    boost::rpc::signed_int si;
    boost::rpc::unsigned_int usi;
    std::vector<std::string> data;
    std::list<int32_t> ldata;
    std::map<std::string,std::string> keys;
    std::vector<char> bdata;
    substruct sub;
};
BOOST_REFLECT( test,(a)(b)(si)(usi)(data)(ldata)(keys)(bdata)(sub) )
BOOST_REFLECT( substruct,(hi)(low) )

int main( int argc, char** argv ) {
    test t;
    t.a = 5;
    t.b = "hello world";
    t.data.push_back( "Hello" );
    t.data.push_back( "World" );
    t.keys["hello"] = "world";
    t.si.value = -1234;
    t.usi.value = 1234;

    t.bdata.insert(t.bdata.begin(), t.b.begin(), t.b.end() );
    std::string jtest = boost::rpc::json::to_json(t);
    std::cerr<<"json: " <<jtest << std::endl;
    std::cerr<< "\n description: \n";
    boost::rpc::description d = boost::rpc::describe_type<test>();
    std::cerr<< "\n...\n";
    std::cerr<<boost::rpc::json::to_json( d );

    std::stringstream ss;
    ss.seekp(0);
    boost::rpc::raw::pack(ss, t);

    int64_t count = 10000000;
    {
    boost::rpc::datastream<size_t> ds;
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    for( int64_t i = 0; i < count; ++i ) {
        ss.seekp(0);
        boost::rpc::raw::pack(ds, t);
    }
    boost::chrono::system_clock::time_point end = boost::chrono::system_clock::now();
    std::cerr<<"size: "<<ds.tellp()<<" time = "<<count / ((end-start).count()/1000000000.0) <<std::endl;
    }

    {
    boost::iostreams::stream<boost::iostreams::null_sink> ds;
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    for( int64_t i = 0; i < count; ++i ) {
        ss.seekp(0);
        boost::rpc::raw::pack(ds, t);
    }
    boost::chrono::system_clock::time_point end = boost::chrono::system_clock::now();
    std::cerr<<"\nsize: "<<ds.tellp();
    std::cerr<<"null_sink time = "<<count / ((end-start).count()/1000000000.0) <<std::endl;
    }

    {
    std::vector<char> tmp(ss.str().size());
    boost::rpc::datastream<char*> ds(&tmp.front(),tmp.size());
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    for( int64_t i = 0; i < count; ++i ) {
        ss.seekp(0);
        boost::rpc::raw::pack(ds, t);
    }
    boost::chrono::system_clock::time_point end = boost::chrono::system_clock::now();
    std::cerr<<"time = "<<count / ((end-start).count()/1000000000.0) <<std::endl;
    }

    {
    std::vector<char> tmp(ss.str().size());
    boost::iostreams::stream<boost::iostreams::array_sink>  
                                        ds(&tmp.front(),tmp.size());
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    for( int64_t i = 0; i < count; ++i ) {
        ss.seekp(0);
        boost::rpc::raw::pack(ds, t);
    }
    boost::chrono::system_clock::time_point end = boost::chrono::system_clock::now();
    std::cerr<<"time = "<<count / ((end-start).count()/1000000000.0) <<std::endl;
    }

    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    for( int64_t i = 0; i < count; ++i ) {
        ss.seekp(0);
        boost::rpc::raw::pack(ss, t);
    }
    boost::chrono::system_clock::time_point end = boost::chrono::system_clock::now();
    std::cerr<<"time = "<<count / ((end-start).count()/1000000000.0) <<std::endl;

    test t2;
    boost::rpc::raw::unpack(ss, t2);
    std::cerr<<"unpacked\n";
    std::cerr<<boost::rpc::json::to_json( t2 );
    return 0;
}
