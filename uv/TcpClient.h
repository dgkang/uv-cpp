/*
   Copyright 2017, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net

   Last modified: 2018-10-9

   Description: https://github.com/wlgq2/libuv_cpp11
*/

#ifndef   UV_TCP_CLIENT_H
#define   UV_TCP_CLIENT_H

#include  <functional>
#include  <memory>

#include  "TcpConnection.h"
#include  "SocketAddr.h"

namespace uv
{

using ConnectCallback =  std::function<void(bool)>  ;
using NewMessageCallback =  std::function<void(const char*,ssize_t)>  ;
using OnConnectClose =  std::function<void()> ;

class TcpClient
{
public:
    TcpClient(EventLoop* loop,bool tcpNoDelay = true);
    virtual ~TcpClient();

    bool isTcpNoDelay();
    void setTcpNoDelay(bool isNoDelay);
    void connect(SocketAddr& addr);

    void onConnect(bool successed);
    void onConnectClose(std::string& name);
    void onMessage(std::shared_ptr<TcpConnection> connection,const char* buf,ssize_t size);
    void close(std::function<void(std::string&)> callback);
    void afterConnectFail();

    void write(const char* buf, unsigned int size, AfterWriteCallback callback = nullptr);
    void writeInLoop(const char* buf, unsigned int size, AfterWriteCallback callback);


    void setConnectCallback(ConnectCallback callback);
    void setMessageCallback(NewMessageCallback callback);
    void setConnectCloseCallback(OnConnectClose callback);

    EventLoop* Loop();

    int appendToBuffer(const char* data, int size);
    int readFromBuffer(Packet& packet);
protected:
    EventLoop* loop_;
private:
    uv_tcp_t* socket_;
    uv_connect_t* connect_;
    SocketAddr::IPV ipv;
    bool tcpNoDelay_;

    ConnectCallback connectCallback_;
    NewMessageCallback onMessageCallback_;
    OnConnectClose onConnectCloseCallback_;

    std::shared_ptr<TcpConnection> connection_;
    void update();
    void runConnectCallback(bool isSuccess);
    void onClose(std::string& name);
};

using TcpClientPtr = std::shared_ptr<uv::TcpClient>;
}
#endif
