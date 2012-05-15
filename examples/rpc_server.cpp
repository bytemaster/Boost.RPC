#include <boost/exception/diagnostic_information.hpp>
#include "calculator.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/cmt/thread.hpp>
#include <boost/rpc/json/tcp_server.hpp>

class CalculatorServer
{
    public:
        CalculatorServer():m_result(0){}
        ~CalculatorServer(){
          slog("");
        }

        std::string name()const            { return "CalculatorServer";  }
        int   exit()                       { ::exit(0);                  }
        double add( double v )             { m_result += v/* + get_num(v)*/;    /*got_result(m_result);*/  return m_result;  }
        double sub( double v )             { m_result -= v;                 return m_result;  }  
        double mult( double v )            { m_result *= v;                 /*got_result(m_result);*/  return m_result;  }
        double div( double v )             { m_result /= v;                 /*got_result(m_result);*/  return m_result;  }
        double add2( double v, double v2 ) { m_result += v + v2;            /*got_result(m_result);*/  return m_result;  }

        double result()const { return m_result; }

        void   set_callback(int c,  const boost::function<std::string(int)>& cb ) {
          m_cb = cb;
          if( m_cb ) std::cerr<<"CB Result: "<<m_cb( 3*c )<<"\n";
        }
        double npt( const named_param_test& p ) {
          int sum = 0;
          if( p.x ) sum += *p.x;
          if( p.y ) sum += -1 * *p.y;
          return sum;
        }
    private:
      boost::function<std::string(int)> m_cb;
        double m_result;
};

boost::shared_ptr<CalculatorServer> create_session() {
  return boost::make_shared<CalculatorServer>();// calc(new CalculatorServer());
}

int main( int argc, char** argv ) {
    if( argc <= 1 )
    {
        std::cerr << "Usage: rpc_server PORT\n";
        return -1;
    }
    using namespace boost;
    try {
        boost::rpc::json::tcp_server<Calculator>  serv( boost::function<boost::shared_ptr<CalculatorServer>()>(create_session), boost::lexical_cast<int>(argv[1]) );
        boost::cmt::exec();
    } catch ( const boost::exception& e )
    {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
    return 0;
}
