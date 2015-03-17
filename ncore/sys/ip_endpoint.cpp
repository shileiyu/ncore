#include <ncore/utils/karma.h>
#include "ip_endpoint.h"

namespace ncore
{


const uint16_t IPEndPoint::kPortAny = 0;
const IPEndPoint IPEndPoint::kAny(IPAddress::kIPAny, IPEndPoint::kPortAny);

IPEndPoint IPEndPoint::FromSockAddr(const sockaddr & sockaddr)
{
    IPEndPoint ep;
    switch(sockaddr.sa_family)
    {
    case AF_INET:
        {
            auto v4 = reinterpret_cast<const sockaddr_in *>(&sockaddr);
            ep.SetAddress(ntohl(v4->sin_addr.S_un.S_addr));
            ep.SetPort(ntohs(v4->sin_port));
        }
        break;
    }
    return ep;
}

IPEndPoint IPEndPoint::FromIPv4(const char * addr)
{
    uint32_t ip = 0;
    uint16_t port = 0;

    if(!addr)
        return IPEndPoint();

    char litera[32] = {0};
    Karma::Copy(litera, addr);
    
    char * spliter = std::strchr(litera, ':');

    if(spliter)
    {
        port = static_cast<uint16_t>(std::atoi(spliter + 1));
        *spliter = 0;
    }

    ip = inet_addr(litera);

    return IPEndPoint(ip, port);
}

IPEndPoint::IPEndPoint()
{
    ep_size_ = sizeof(ep_v6_);
    memset(&ep_v6_, 0, sizeof(ep_v6_));
}

IPEndPoint::IPEndPoint(uint32_t ip, uint16_t port)
{  
    memset(&ep_v6_, 0, sizeof(ep_v6_));
    ep_.sa_family = AF_INET;
    ep_v4_.sin_addr.S_un.S_addr = htonl(ip);
    ep_v4_.sin_port = htons(port);
    ep_size_ = sizeof(ep_v4_);
}

IPEndPoint::~IPEndPoint()
{
}

void IPEndPoint::SetAddress(uint32_t ip)
{
    ep_.sa_family = AF_INET;
    ep_v4_.sin_addr.S_un.S_addr = htonl(ip);
    ep_size_ = sizeof(ep_v4_);
}

void IPEndPoint::SetPort(uint16_t port)
{
    ep_v4_.sin_port = htons(port);
}

IPAddress IPEndPoint::Host() const
{
    switch(ep_.sa_family)
    {
    case AF_INET:
        uint32_t addr_v4 = ntohl(ep_v4_.sin_addr.S_un.S_addr);
        return IPAddress(addr_v4);
    }
    return IPAddress();
}

uint16_t IPEndPoint::Port() const
{
    return ntohs(ep_v4_.sin_port);
}

AddressFamily IPEndPoint::AddressFamily() const
{
    return static_cast<ncore::AddressFamily>(ep_.sa_family);
}

}