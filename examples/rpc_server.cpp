#include <boost/exception/diagnostic_information.hpp>
#include "calculator.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/cmt/thread.hpp>
#include <boost/rpc/json/server.hpp>
#include <boost/rpc/json/tcp/server.hpp>

class CalculatorServer
{
    public:
        CalculatorServer():m_result(0){}

        std::string name()const            { return "CalculatorServer";  }
        int   exit()                       { ::exit(0);                  }
        double add( double v )             { return m_result += v;       }
        double sub( double v )             { return m_result -= v;       }
        double mult( double v )            { return m_result *= v;       }
        double div( double v )             { return m_result /= v;       }
        double add2( double v, double v2 ) { return m_result += v + v2;  }

        double result()const { return m_result; }

    private:
        double m_result;
};

int main2( int argc, char** argv ) {
    if( argc <= 1 )
    {
        std::cerr << "Usage: rpc_server PORT\n";
        return -1;
    }
    using namespace boost;
    try {

        boost::shared_ptr<CalculatorServer> calc(new CalculatorServer());
        boost::rpc::json::server<Calculator> calcs(calc);
        boost::rpc::json::tcp::listen( atoi(argv[1]), boost::bind(&boost::rpc::json::server<Calculator>::add_connection, &calcs, _1) );

        boost::cmt::usleep(1000000ll*60*60);
    } catch ( const boost::exception& e )
    {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
    return 0;
}

int main( int argc, char** argv )
{
    boost::cmt::async( boost::bind(main2, argc, argv) );
    boost::cmt::exec();
    return 0;
}



