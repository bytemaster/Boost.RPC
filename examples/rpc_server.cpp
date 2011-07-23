#include <boost/exception/diagnostic_information.hpp>
#include "calculator.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/cmt/thread.hpp>
#include <boost/cmt/signals.hpp>
#include <boost/rpc/json/server.hpp>
#include <boost/rpc/json/tcp/server.hpp>

class CalculatorServer
{
    public:
        CalculatorServer():m_result(0){}

        std::string name()const            { return "CalculatorServer";  }
        int   exit()                       { ::exit(0);                  }
        double add( double v )             { m_result += v + get_num(v);    got_result(m_result);  return m_result;  }
        double sub( double v )             { m_result -= v;                 got_result(m_result);  return m_result;  }  
        double mult( double v )            { m_result *= v;                 got_result(m_result);  return m_result;  }
        double div( double v )             { m_result /= v;                 got_result(m_result);  return m_result;  }
        double add2( double v, double v2 ) { m_result += v + v2;            got_result(m_result);  return m_result;  }

        double result()const { return m_result; }

        boost::signal<void(double)> got_result;
        boost::signal<int(double)>  count;
        boost::signal<float(double)>  get_num;

    private:
        double m_result;
};
void print_result( double r ) {
    std::cerr<<"result: "<<r<<std::endl;
}

int do_count( double v ) { return v*v; }
int do_count2( double v ) { return v*v*v; }

void create_session( const boost::shared_ptr<CalculatorServer>& calc, const boost::rpc::json::connection::ptr& con ) {
    std::cerr<<"new connection\n";
    boost::rpc::json::server<Calculator>::ptr s(new boost::rpc::json::server<Calculator>( calc, con ));
    boost::cmt::wait( con->disconnected );
}

int main2( int argc, char** argv ) {
    if( argc <= 1 )
    {
        std::cerr << "Usage: rpc_server PORT\n";
        return -1;
    }
    using namespace boost;
    try {

        boost::shared_ptr<CalculatorServer> calc(new CalculatorServer());
        calc->got_result.connect(print_result);
        calc->count.connect(do_count);
        calc->count.connect(do_count2);

        boost::rpc::json::tcp::listen( atoi(argv[1]), boost::bind(create_session, calc, _1) );

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



