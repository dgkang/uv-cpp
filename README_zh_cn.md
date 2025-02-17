# uv-cpp
<a href="https://github.com/wlgq2/libuv_cpp11/releases"><img src="https://img.shields.io/github/release/wlgq2/libuv_cpp11.svg" alt="Github release"></a>
[![Platform](https://img.shields.io/badge/platform-%20%20%20%20Linux,%20Windows-green.svg?style=flat)](https://github.com/wlgq2/libuv_cpp11)
[![License](https://img.shields.io/badge/license-%20%20MIT-yellow.svg?style=flat)](LICENSE)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)


<br>对libuv的C++11风格的跨平台封装库，用于线上项目。跑过压测，很稳定，正确使用接口情况下，未发现core dump或内存泄漏。</br>
## 特性
** **
* C++11风格回调函数：非C语言函数回调，支持非静态类成员函数及lambda。
* TCP及UDP相关类封装：`TcpServer`、`TcpClient`、`TcpConnection`、`TcpAccept`、`Udp`。
* `Timer`及`TimerWheel`：定时器及时间复杂度为O(1)的心跳超时踢出机制。
* `Async`：异步机制封装。相对于原生libuv async接口，优化了调用多次可能只运行一次的问题。由于libuv几乎所有api都非线程安全，建议使用writeInLoop接口代替直接write（writeInLoop会检查当前调用的线程，如果在loop线程中调用则直接write，否则把write加到loop线程中执行）。
* libuv信号封装。   
* `Packet`与`PacketBuffer`：包与缓存，发送/接受包，用于解决TCP残包/粘包问题，由ListBuffer和CycleBuffer两种实现，可通过宏配置（前者空间友好，后者时间友好）。
* Log日志输出接口，可绑定至自定义Log库。
** **
## 简单性能测试
单线程1k字节ping-pong。
<br>环境：Intel Core i5 6402 + ubuntu14.04.5 + gcc5.5.0 + libuv1.22.0 + O2优化</br>

   libuv_cpp | no use PacketBuffer|CycleBuffer|ListBuffer|
:---------:|:--------:|:--------:|:--------:|
次/秒     | 192857 |141487|12594|

## 第一个例程
```C++
#include <iostream>
#include <uv/uv11.h>


int main(int argc, char** args)
{
    //event's loop
    uv::EventLoop* loop = uv::EventLoop::DefalutLoop();

    //Tcp Server
    uv::SocketAddr serverAddr("0.0.0.0", 10002, uv::SocketAddr::Ipv4);
    uv::TcpServer server(loop, serverAddr);
    server.setMessageCallback(
        [](std::shared_ptr<uv::TcpConnection> conn, const char* data , ssize_t size)
    {
        std::cout << std::string(data, size) << std::endl;
        conn->write(data, size,nullptr);
    });
    server.start();

    //Tcp Client
    uv::TcpClient client(loop);
    client.setConnectCallback(
        [&client](bool isSuccess)
    {
        if (isSuccess)
        {
            char data[] = "hello world!";
            client.write(data, sizeof(data));
        }
    });
    client.connect(serverAddr);
       
    loop->run();
}

```

** **
<br>**！对于诸如`uv::Timer`,`uv::TcpClient`等对象的释放需要调用close接口并在回调函数中释放对象,否则可能会出错。**</br>
<br>**！切勿在Loop线程外创建注册该Loop下的事件相关对象（`uv::TcpClient`，`uv::TcpServer`，`uv::Timer`……），建议每个Loop都绑定独立线程运行。**</br>
<br>一点微小的工作。</br>
