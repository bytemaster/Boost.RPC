#ifndef _BITCOIN_HPP_
#define _BITCOIN_HPP_
#include <boost/asio.hpp>
#include <boost/filesystem/path.hpp>
#include <mace/stub/ptr.hpp>

namespace bitcoin {
    namespace detail { class client; }

struct address_info {
    bool isvalid;
    std::string address;
    bool ismine;
    std::string account;
};


struct server_info {
    uint64_t     version;
    uint64_t     balance;
    uint64_t     blocks;
    uint64_t     connections;
    std::string  proxy;
    bool         generate;
    int32_t      genproclimit;
    double       difficulty;
    double       hashespersec;
    bool         testnet;
    uint64_t     keypoololdest;
    uint64_t     paytxfee;
    std::string  errors;
};


class client {
    public:
        std::string               backupwallet( const std::string& destination );
        std::string               getaccount( const std::string& address );
        std::string               getaccountaddress( const std::string& account );
        std::vector<std::string>  getaddressesbyaccount( const std::string& account );
        uint64_t                  getbalance( const std::string& account = "", uint32_t minconf = 1 );

        uint32_t                  getblockcount();
        uint32_t                  getblocknumber();
        uint32_t                  getconnectioncount();
        double                    getdifficulty();
        bool                      getgenerate();
        server_info               getinfo();
                                  
        uint64_t                  getreceivedbyaddress( const std::string& address, uint32_t minconf = 1 );
        uint64_t                  getreceivedbyaccount( const std::string& account, uint32_t minconf = 1 );

        std::string  getnewaddress( const std::string& account = "" );
        void         setaccount( const std::string& address, const std::string& account );
        address_info validateaddress( const std::string& address );

    private:
        detail::client* my;

        // create address... 
        // query balance...
};

} // namespace bitcoin
MACE_REFLECT( bitcoin::address_info, (isvalid)(address)(ismine)(account) )

MACE_REFLECT( bitcoin::server_info, 
    (version)
    (balance)
    (blocks)
    (connections)
    (proxy)
    (generate)
    (genproclimit)
    (difficulty)
    (hashespersec)
    (testnet)
    (keypoololdest)
    (paytxfee)
    (errors)
)

MACE_STUB( bitcoin::client,
  (backupwallet)
  (getaccount)
  (getaccountaddress)
  (getaddressesbyaccount)
  (getbalance)
  (getblockcount)
  (getblocknumber)
  (getconnectioncount)
  (getdifficulty)
  (getgenerate)
  (getinfo)
  (getreceivedbyaddress)
  (getreceivedbyaccount)
  (getnewaddress)
  (setaccount)
  (validateaddress)
)

#endif
