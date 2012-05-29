include_directories( include )
include_directories( ../json/include )
include_directories( ../cmt/include )
include_directories( ../stub/include )
include_directories( ../reflect/include )
include_directories( ../context/include )
include_directories( ../atomic/include )
include_directories( ../network/include )

add_subdirectory( examples )

SET( libraries  mace_cmt )
SET( sources 
        src/json/connection.cpp
        src/json/tcp_connection.cpp
        src/json/client_base.cpp
        src/json/value.cpp
        src/json/tcp_server.cpp
        src/json/http_server.cpp
        src/json/http_connection.cpp

        src/http/file_handler.cpp
        src/http/path_handler.cpp
        src/http/mime_types.cpp
        src/http/reply.cpp
        src/http/request_parser.cpp
        src/http/server.cpp
             )

install( DIRECTORY include/boost/rpc DESTINATION include/boost )

SETUP_LIBRARY( mace_rpc SOURCES ${sources}
                         LIBRARIES  ${libraries}
                         LIBRARY_TYPE STATIC )
 

