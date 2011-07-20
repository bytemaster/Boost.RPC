SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    boost_cmt
    boost_rpc
    boost_context 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( rpcs  rpc_server.cpp  )
target_link_libraries( rpcs ${libraries} )
add_executable( rpcc  rpc_client.cpp  )
target_link_libraries( rpcc ${libraries} )

add_executable( jsonc  json_client.cpp  )
target_link_libraries( jsonc ${libraries} )
add_executable( jsons  json_echo_server.cpp  )
target_link_libraries( jsons ${libraries} )
