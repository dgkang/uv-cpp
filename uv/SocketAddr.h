/*
Copyright 2017, orcaer@yeah.net  All rights reserved.

Author: orcaer@yeah.net

Last modified: 2019-8-5

Description: https://github.com/wlgq2/libuv_cpp11
*/

#ifndef    UV_SOCKET_ADDR_H
#define    UV_SOCKET_ADDR_H

#include <string>

#include <uv.h>

namespace uv
{ 

class  SocketAddr
{
public:
    enum IPV
    {
        Ipv4 = 0,
        Ipv6
    };

    SocketAddr(const std::string&& ip, unsigned short port, IPV ipv = Ipv4);
    SocketAddr(const std::string& ip, unsigned short port, IPV ipv = Ipv4);
    SocketAddr(const sockaddr* addr, IPV ipv = Ipv4);

    const sockaddr* Addr();
    void toStr(std::string& str);
    std::string toStr();

    IPV Ipv();

    static  void AddrToStr(uv_tcp_t* client, std::string& addrStr, IPV ipv = Ipv4);
    static  uint16_t  GetIpAndPort(const sockaddr_storage* addr, std::string& out, IPV ipv = Ipv4);

private:
    std::string ip_;
    unsigned short port_;
    const IPV ipv_;
    sockaddr_in ipv4_;
    sockaddr_in6 ipv6_;
};

}
#endif
