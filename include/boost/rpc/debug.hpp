#ifndef _BOOST_RPC_DEBUG_HPP_
#define _BOOST_RPC_DEBUG_HPP_
#define  COLOR_CONSOLE 1
#include <boost/format.hpp>
#include <iostream>
#include <boost/rpc/console_defines.h>

namespace boost { namespace rpc { namespace detail {

    void log( const char* color, const char* file, uint32_t line, const char* method, const char* text )
    {
        std::cerr<<color<<file<<":"<<line<<" "<<method<<"] "<<text<< CONSOLE_DEFAULT << std::endl; 
    }
    template<typename P1>
    void log( const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1 )
    {
        std::cerr<<color<<file<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2>
    void log( const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2 )
    {
        std::cerr<<color<<file<<":"<<line<<" "<<method<<"] "<< boost::format(format) %p1 %p2 << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3>
    void log( const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3 )
    {
        std::cerr<<color<<file<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3) << CONSOLE_DEFAULT << std::endl;
    }

} } } // boost::rpc::detail
#define dlog(...) boost::rpc::detail::log( CONSOLE_DEFAULT, __FILE__, __LINE__, __func__, __VA_ARGS__ )
#define dlog(...) boost::rpc::detail::log( CONSOLE_DEFAULT, __FILE__, __LINE__, __func__, __VA_ARGS__ )
#define elog(...) boost::rpc::detail::log( CONSOLE_RED,     __FILE__, __LINE__, __func__, __VA_ARGS__ )
#define wlog(...) boost::rpc::detail::log( CONSOLE_BROWN,   __FILE__, __LINE__, __func__, __VA_ARGS__ )

#endif 
