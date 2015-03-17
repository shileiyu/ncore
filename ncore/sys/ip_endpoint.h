#ifndef NCORE_SYS_IP_ENDPOINT_H_
#define NCORE_SYS_IP_ENDPOINT_H_

#include "ip_address.h"

namespace ncore
{


class IPEndPoint
{
public:
    static const uint16_t kPortAny;
    static const IPEndPoint kAny;
public:
    static IPEndPoint FromSockAddr(const sockaddr & sockaddr);
    //"127.0.0.1:80"
    static IPEndPoint FromIPv4(const char * addr);
public:
    IPEndPoint();
    IPEndPoint(uint32_t ip, uint16_t port);//初始化成ipv4地址
    ~IPEndPoint();

    void SetAddress(uint32_t ip);
    void SetPort(uint16_t port);
 
    IPAddress Host() const;
    uint16_t Port() const;
    AddressFamily AddressFamily() const;
private:
    size_t ep_size_;
    union
    {
        sockaddr ep_;
        sockaddr_in ep_v4_;
        sockaddr_in6 ep_v6_;
    };

    friend class Socket;
};


}

#endif