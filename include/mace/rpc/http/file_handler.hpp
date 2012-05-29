//
// file_handler.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MACE_RPC_HTTP_FILE_HANDLER_HPP
#define MACE_RPC_HTTP_FILE_HANDLER_HPP

#include <mace/rpc/http/request.hpp>
#include <mace/rpc/http/reply.hpp>
#include <string>

namespace mace { namespace rpc { namespace http {

    struct reply;
    struct request;

    /// The common handler for all incoming requests.
    class file_handler
    {
    public:
      /// Construct with a directory containing files to be served.
      explicit file_handler(const std::string& doc_root);

      /// Handle a request and produce a reply.
      bool operator()(const request& req, const std::string& path, reply& rep);

    private:
      /// The directory containing the files to be served.
      std::string doc_root_;

    };
    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    bool url_decode(const std::string& in, std::string& out);

} } } // boost::rpc::http

#endif // MACE_RPC_HTTP_FILE_HANDLER_HPP
