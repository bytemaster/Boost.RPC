SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    mace_rpc
    boost_context 
    pthread
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

IF(UNIX) 
    set( libraries ${libraries} pthread )
ENDIF(UNIX)

#add_executable( jsonv  jsonv.cpp  )
#target_link_libraries( jsonv ${libraries} )

add_executable( rpcs  rpc_server.cpp  )
target_link_libraries( rpcs ${libraries} )

add_executable( rpcc  rpc_client.cpp  )
target_link_libraries( rpcc ${libraries} )

add_executable( benchc  benchc.cpp  )
target_link_libraries( benchc ${libraries} )

add_executable( jsonhttpd  jsonhttpd.cpp  )
target_link_libraries( jsonhttpd ${libraries} cppnetlib-uri cppnetlib-server-parsers )
add_executable( jsonhttpc  jsonhttpc.cpp  )
target_link_libraries( jsonhttpc ${libraries} cppnetlib-uri cppnetlib-server-parsers cppnetlib-client-connections)

#add_executable( jsonc  json_client.cpp  )
#target_link_libraries( jsonc ${libraries} )
#add_executable( jsons  json_echo_server.cpp  )
#target_link_libraries( jsons ${libraries} )
