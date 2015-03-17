#ifndef NCORE_SYS_NETWORK_DEFINE_H_
#define NCORE_SYS_NETWORK_DEFINE_H_

#include <ncore/ncore.h>

namespace ncore
{

enum AddressFamily
{
    kUnknownAddressFamily = -1,
    kUnspecificAddressFamily = AF_UNSPEC,
    kInterNetwork = AF_INET,
    kInterNetworkV6 = AF_INET6,
};

enum ProtocolType
{
    kTCP = IPPROTO_TCP,
    kUDP = IPPROTO_UDP,
};

enum SocketType
{
    kStream = SOCK_STREAM,
    kDgram = SOCK_DGRAM,
};

enum SocketShutdown
{
    kReceive = SD_RECEIVE,
    kSend = SD_SEND,
    kBoth = SD_BOTH,
};

}

#endif