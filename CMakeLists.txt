# Distributed under the Boost Software License, Version 1.0.
# See http://www.boost.org/LICENSE_1_0.txt

boost_project("RPC"
  AUTHORS
    "Daniel Larimer"
  DESCRIPTION
    "Generic RPC library proposal for Boost"
  )

if( ENABLE_EXAMPLES )
    option( ENABLE_IDL_EXAMPLES "Build Boost.IDL Examples" ON )
    if( ENABLE_RPC_EXAMPLES )
        boost_add_executable( rpc_protobuf  SOURCES examples/proto_buf.cpp   LINK_BOOST_LIBRARIES )
        boost_add_executable( rpc_client    SOURCES examples/rpc_client.cpp  LINK_BOOST_LIBRARIES system serialization)
        boost_add_executable( rpc_server    SOURCES examples/rpc_server.cpp  LINK_BOOST_LIBRARIES system serialization)
    endif( ENABLE_RPC_EXAMPLES )
endif( ENABLE_EXAMPLES )

add_subdirectory(doc)
add_subdirectory(include)
