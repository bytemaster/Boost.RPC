Boost.RPC - Remote Procedure Call library
---------------------------------------

The goal of this library is to enable seamless remote procedure
calls between C++ objects in different processes and machines.

The Boost RPC library will leverage Boost.Reflect to abstract
the difference between classes on remote machines and local machines.

The library will aim to support any number of different RPC protocols 
and will provide built-in support for Google Protocol Buffers,
JSON RPC, XML RPC, and any Boost.Serialization archive formats.

### Warning ###
    This code is in early alpha phase.  I welcome feedback and help in
    defining the scope of this library.  

### Notice ###

    This library is not part of the official Boost C++ library, but
    is written, to the best of my ability, to follow the best practices
    established by the Boost community and with the hope of being 
    considered for inclusion with a future Boost release.

### Requirements ###

This library is being written with the following requirements in mind:

 - C++ as primary language for both sides of the RPC.
 - No code generation required for C++/C++ interoperability
 - Transparently support Google Protocol Buffers, JSON-RPC, XML-RPC, and
    Boost.Serialization.
 - Generate .proto file from reflected type-info to enable non-C++
    programs to interact.
 - Network layer is abstraction allowing 3rd parties to provide TCP/IP,
    UDP, UDT, or other transport mechinism. 

### Status ###

A working json RPC library has been implemented using TCP as the transport.

JSON RPC Client:

    // calculator.hpp
    struct Service {
        std::string name()const;
    };
    struct Calculator : Service {
        double add( double v );           
        double add2( double v, double v2 );
        double sub( double v );           
        double mult( double v );           
        double div( double v );           
        double result()const;
    };

    BOOST_REFLECT_ANY( Service, BOOST_PP_SEQ_NIL, (name)(exit) )
    BOOST_REFLECT_ANY( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result) )

    // client.cpp
    rpc::json::tcp::connection::ptr con( new boost::rpc::json::tcp::connection() );
    con->connect( host, port );
    boost::rpc::json::client<Calculator> calc(con);

    // make the call
    double r = calc->add2( 5, 6 );

JSON RPC Server:
    #include "calculator.hpp"

    // define your service
    class CalculatorServer {
        public:
            CalculatorServer():m_result(0){}

            std::string name()const            { return "CalculatorServer";  }
            double add( double v )             { return m_result += v;       }
            double sub( double v )             { return m_result -= v;       }
            double mult( double v )            { return m_result *= v;       }
            double div( double v )             { return m_result /= v;       }
            double add2( double v, double v2 ) { return m_result += v + v2;  }

            double result()const { return m_result; }

        private:
            double m_result;
    };
    // create your service
    CalculatorServer* calc =new CalculatorServer());

    // create a server of type calculator, see RPC Client
    boost::rpc::json::server<Calculator> calc_serv(calc);

    // listen on a tcp port for new json tcp connections
    boost::rpc::json::tcp::listen( port, 
        boost::bind(&boost::rpc::json::server<Calculator>::add_connection, 
                    &calc_serv, _1) );


Things to notice about the above examples is that the CalculatorServer class is unrelated to
the Calculator interface other than that it implements the methods defined by Calculator and
its base class Service.  The calc_serv will work with any type that provides those methods. 


### Dependencies ###
 - Boost.CMT is used for asynchronous calls/multi-tasking
 - Boost.Context is used by Boost.CMT
 - Boost.Atomic is used by Boost.CMT
 - Boost.Move is used by Boost.Context
 - Boost.Reflect will be used to provide reflection/abstraction of class
    interfaces and message structures.
 - Boost.Optional will be used for optional message fields.
 - Boost.Function will be used for generic callbacks/delegates.

### Google Protocol Buffers with Boost.RPC ###

Google Protocol Buffers are a very nice, effecient, means of defining
a message such that it is both forward and backward compatible. Google
accomplished this feat using three simple ideas:

- Key/Value Pairs
- Size information stored for each field
- Base 128 varint encoding

Google Protocol Buffers also defines a ".proto" language that is
combined with code generation to define serialize helper classes
in multiple languages.  This works for many people, but for some
code-generation and linking against a library is too 'heavy weight'
and 'messy'.  

I wanted to have the benefits of protocol buffers, without the
pitfalls of using code generation.  I also wanted to leverage other
benefits of Boost.Reflect to enable reflection as well as serialization
in other formats such as JSON.

My solution looks like this:

    struct Person
    {
        required<string>  name;
        required<int32_t> id;
        optional<string>  email;

        enum PhoneType
        {
            MOBILE = 0;
            HOME = 1;
            WORK = 2;
        };

        struct PhoneNumber
        {
            PhoneNumber():type(HOME){}

            required<string>     number;
            optional<PhoneType>  type;
        };

        std::vector<PhoneNumber> phone;
    };

    BOOST_REFLECT( Person, BOOST_IDL_BASE, (name)(id)(email)(phone) )
    BOOST_REFLECT( Person::PhoneNumber, BOOST_IDL_BASE, (number)(type) )


Then you can use the following methods to serialize the message:

    std::vector<char> buf;
    rpc::protocol_buffer::pack( buf, Person() );

If any of the required fields were not set, then packing the buffer would throw
an exception.

You can unpack a message like this:

    std::vector<char> buf;
    Person p;
    rpc::protocol_buffer::unpack( buf, p );

    // check for missing required fields
    rpc::protocol_buffer::validate( p );



### License ###

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.


