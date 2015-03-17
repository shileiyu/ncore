#ifndef NCORE_SYS_IP_ADDRESS_H_
#define NCORE_SYS_IP_ADDRESS_H_

#include <ncore/ncore.h>
#include "network_define.h"

namespace ncore
{

class IPAddress
{
public:
    static const uint32_t kIPAny;
    static const uint32_t kIPBroadcast;
    static const uint32_t kIPLoopback;
public:
    IPAddress();
    IPAddress(int32_t addr);
    IPAddress(uint32_t addr);

    uint32_t Address() const;
    void SetAddress(uint32_t addr);

    AddressFamily AddressFamily() const;
private:
    size_t addr_size_;
    union 
    {
        in_addr addr_v4_;
        in6_addr addr_v6_;
    };
};


}

#endif //NCORE_SYS_IP_ADDRESS_H_