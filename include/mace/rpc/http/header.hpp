//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MACE_RPC_HTTP_HEADER_HPP
#define MACE_RPC_HTTP_HEADER_HPP

#include <string>

namespace mace { namespace rpc { namespace http {

    struct header {
      std::string name;
      std::string value;
    };

} } } // mace::rpc::http

#endif // MACE_RPC_HTTP_HEADER_HPP
