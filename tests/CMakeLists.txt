SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    boost_cmt
    boost_rpc
    boost_context 
    ${Boost_CHRONO_LIBRARY}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

IF(UNIX) 
    set( libraries ${libraries} pthread )
ENDIF(UNIX)

#add_executable( rpc_raw  raw.cpp  )
#target_link_libraries( rpc_raw ${libraries} )
