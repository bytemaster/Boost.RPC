#include <mace/rpc/json/http_server.hpp>
#include <iostream>
#include "calculator.hpp"

class CalculatorServer {
    public:
        CalculatorServer():m_result(0){}
        ~CalculatorServer(){
          slog("");
        }

        std::string name()const            { return "CalculatorServer";  }
        int   exit()                       { ::exit(0);                  }
        double add( double v )             { m_result += v;      return m_result;  }
        double sub( double v )             { m_result -= v;      return m_result;  }  
        double mult( double v )            { m_result *= v;      return m_result;  }
        double div( const calc_str& v )    {                     return m_result;  }
        double add2( double v, double v2 ) { m_result += v + v2; return m_result;  }

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

int main(int argc, char * argv[]) {
  if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " address port" << std::endl;
      return 1;
  }

  try {
    mace::rpc::json::http_server server( argv[1], boost::lexical_cast<int>(argv[2]) );
    server.add_service<Calculator>( "/calculator", create_session() );
    mace::cmt::exec();
  }
  catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return 1;
  }
  
  return 0;
}
//]
