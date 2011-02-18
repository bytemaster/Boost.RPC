Boost.RPC - Remote Procedure Call library
---------------------------------------

The goal of this library is to enable seamless remote procedure
calls between C++ objects in different processes and machines.

The Boost RPC library will leverage Boost.IDL to abstract
the difference between classes on remote machines and local machines.

The library will aim to support any number of different RPC protocols 
and will provide built-in support for Google Protocol Buffers and
JSON RPC.


### Google Protocol Buffers ###

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
benefits of Boost.IDL to enable reflection as well as serialization
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

    BOOST_IDL_REFLECT( Person, BOOST_IDL_BASE, (name)(id)(email)(phone) )
    BOOST_IDL_REFLECT( Person::PhoneNumber, BOOST_IDL_BASE, (number)(type) )


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






### Notice ###

    This library is not part of the official Boost C++ library, but
    is written, to the best of my ability, to follow the best practices
    established by the Boost community and with the hope of being 
    considered for inclusion with a future Boost release.
