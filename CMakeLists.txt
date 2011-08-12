include_directories( include )
include_directories( ../json/include )
include_directories( ../cmt/include )
include_directories( ../reflect/include )
include_directories( ../context/include )
include_directories( ../move/include )
include_directories( ../atomic/include )

add_subdirectory( examples )
add_subdirectory( tests )

SET( libraries boost_json boost_cmt )
SET( sources 
    src/json/tcp/connection.cpp
    src/json/connection.cpp
    src/json/value.cpp
    src/json/tcp/server.cpp
             )

install( DIRECTORY include/boost/rpc DESTINATION include/boost )

SETUP_LIBRARY( boost_rpc SOURCES ${sources}
                         LIBRARIES  ${libraries}
                         LIBRARY_TYPE STATIC )
 

#add_executable( jtest examples/json.cpp  )
#target_link_libraries( jtest boost_cmt
#                        boost_context 
#                        boost_rpc 
#                       ${Boost_THREAD_LIBRARY} 
#                       ${Boost_SERIALIZATION_LIBRARY} 
#                       ${Boost_ASIO_LIBRARY} 
#                       ${Boost_SYSTEM_LIBRARY} 
#                       ${Boost_SIGNALS_LIBRARY})


#add_executable( crpc examples/rpc_client.cpp  )
#target_link_libraries( crpc boost_rpc boost_cmt
#                        boost_context 
#                       ${Boost_THREAD_LIBRARY} 
#                       ${Boost_SERIALIZATION_LIBRARY} 
#                       ${Boost_ASIO_LIBRARY} 
#                       ${Boost_SYSTEM_LIBRARY} 
#                       ${Boost_SIGNALS_LIBRARY})

