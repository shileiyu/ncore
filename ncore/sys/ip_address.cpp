#include "ip_address.h"

namespace ncore
{


const uint32_t IPAddress::kIPAny = 0x00000000;
const uint32_t IPAddress::kIPBroadcast = 0xFFFFFFFF;
const uint32_t IPAddress::kIPLoopback = 0x7F000001;

IPAddress::IPAddress()
{
    memset(&addr_v6_, 0, sizeof(addr_v6_));
}

IPAddress::IPAddress(uint32_t addr)
{
    memset(&addr_v6_, 0, sizeof(addr_v6_));
    SetAddress(addr);
}

uint32_t IPAddress::Address() const
{
    return ntohl(addr_v4_.S_un.S_addr);
}

void IPAddress::SetAddress(uint32_t addr)
{
    addr_v4_.S_un.S_addr = htonl(addr);
    addr_size_ = sizeof(addr_v4_);
}

AddressFamily IPAddress::AddressFamily() const
{
    if(addr_size_ == sizeof(addr_v4_))
        return AddressFamily::kInterNetwork;
    else if(addr_size_ == sizeof(addr_v6_))
        return AddressFamily::kInterNetworkV6;

    return AddressFamily::kUnspecificAddressFamily;
}


}