/*
Copyright 2017, orcaer@yeah.net  All rights reserved.

Author: orcaer@yeah.net

Last modified: 2017-11-8

Description: https://github.com/wlgq2/libuv_cpp11
*/

#include "TcpConnection.h"
#include "TcpServer.h"
#include "Async.h"
#include "LogWriter.h"

using namespace std;
using namespace std::chrono;
using namespace uv;

struct WriteReq
{
    uv_write_t req;
    uv_buf_t buf;
    AfterWriteCallback callback;
};

struct WriteArgs
{
    WriteArgs(shared_ptr<TcpConnection> conn = nullptr, const char* buf = nullptr, ssize_t size = 0, AfterWriteCallback callback = nullptr)
        :connection(conn),
        buf(buf),
        size(size),
        callback(callback)
    {

    }
    shared_ptr<TcpConnection> connection;
    const char* buf;
    ssize_t size;
    AfterWriteCallback callback;
};

TcpConnection:: ~TcpConnection()
{
    delete handle_;
}

TcpConnection::TcpConnection(EventLoop* loop, std::string& name, uv_tcp_t* client, bool isConnected)
    :name_(name),
    connected_(isConnected),
    loop_(loop),
    handle_(client),
    onMessageCallback_(nullptr),
    onConnectCloseCallback_(nullptr),
    closeCompleteCallback_(nullptr)
{
    handle_->data = static_cast<void*>(this);
    ::uv_read_start((uv_stream_t*)client,
        [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        buf->base = new char[suggested_size];
#if _MSC_VER
        buf->len = (ULONG)suggested_size;
#else
        buf->len = suggested_size;
#endif
    },
        &TcpConnection::onMesageReceive);
}




void TcpConnection::onMessage(const char* buf, ssize_t size)
{
    if (onMessageCallback_)
        onMessageCallback_(shared_from_this(), buf, size);
}

void TcpConnection::onSocketClose()
{
    if (onConnectCloseCallback_)
        onConnectCloseCallback_(name_);
}

void TcpConnection::close(std::function<void(std::string&)> callback)
{
    closeCompleteCallback_ = callback;
    if (::uv_is_active((uv_handle_t*)handle_)) 
    {
        ::uv_read_stop((uv_stream_t*)handle_);
    }
    if (::uv_is_closing((uv_handle_t*)handle_) == 0)
    {
        //libuv 在loop轮询中会检测关闭句柄，delete会导致程序异常退出。
        ::uv_close((uv_handle_t*)handle_,
            [](uv_handle_t* handle)
        {
            auto connection = static_cast<TcpConnection*>(handle->data);
            connection->CloseComplete();
        });
    }
    else
    {
        CloseComplete();
    }
}

int TcpConnection::write(const char* buf, ssize_t size, AfterWriteCallback callback)
{
    int rst;
    if (connected_)
    {
        WriteReq* req = new WriteReq;
        req->buf = uv_buf_init(const_cast<char*>(buf), static_cast<unsigned int>(size));
        req->callback = callback;
        rst = ::uv_write((uv_write_t*)req, (uv_stream_t*)handle_, &req->buf, 1,
            [](uv_write_t *req, int status)
        {
            WriteReq *wr = (WriteReq*)req;
            if (nullptr != wr->callback)
            {
                struct WriteInfo info;
                info.buf = const_cast<char*>(wr->buf.base);
                info.size = wr->buf.len;
                info.status = status;
                wr->callback(info);
            }
            //delete [] (wr->buf.base);
            delete wr;
        });
        if (0 != rst)
        {
            uv::LogWriter::Instance()->error(std::string("write data error:"+std::to_string(rst)));
            if (nullptr != callback)
            {
                struct WriteInfo info = { rst,const_cast<char*>(buf),static_cast<unsigned long>(size) };
                callback(info);
            }
        }
    }
    else
    {
        rst = -1;
        if (nullptr != callback)
        {
            struct WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),static_cast<unsigned long>(size) };
            callback(info);
        }
    }
    return rst;
}

void TcpConnection::writeInLoop(const char* buf, ssize_t size, AfterWriteCallback callback)
{
    loop_->runInThisLoop(
        [this,buf,size, callback]()
    {
        write(buf, size, callback);
    });
}


void TcpConnection::setElement(shared_ptr<ConnectionElement> conn)
{
    element_ = conn;
}

std::weak_ptr<ConnectionElement> TcpConnection::Element()
{
    return element_;
}

void  TcpConnection::onMesageReceive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
    auto connection = static_cast<TcpConnection*>(client->data);
    if (nread > 0)
    {
        connection->onMessage(buf->base, nread);
        delete[](buf->base);
    }
    else if (nread < 0)
    {
        connection->setConnectStatus(false);
        uv::LogWriter::Instance()->error( uv_err_name((int)nread));
        delete[](buf->base);

        if (nread != UV_EOF)
        {
            connection->onSocketClose();
            return;
        }

        uv_shutdown_t* sreq = new uv_shutdown_t;
        sreq->data = static_cast<void*>(connection);
        ::uv_shutdown(sreq, (uv_stream_t*)client,
            [](uv_shutdown_t* req, int status)
        {
            auto connection = static_cast<TcpConnection*>(req->data);
            connection->onSocketClose();
            delete req;
        });
    }
    else
    {
        /* Everything OK, but nothing read. */
        delete[](buf->base);
    }

}

void uv::TcpConnection::setMessageCallback(OnMessageCallback callback)
{
    onMessageCallback_ = callback;
}

void uv::TcpConnection::setConnectCloseCallback(OnCloseCallback callback)
{
    onConnectCloseCallback_ = callback;
}

void uv::TcpConnection::CloseComplete()
{
    if (closeCompleteCallback_)
    {
        closeCompleteCallback_(name_);
    }
}

void uv::TcpConnection::setConnectStatus(bool status)
{
    connected_ = status;
}

bool uv::TcpConnection::isConnected()
{
    return connected_;
}

std::string & uv::TcpConnection::Name()
{
    return name_;
}

int uv::TcpConnection::appendToBuffer(const char* data, int size)
{
    return buffer_.append(data, size);
}

int uv::TcpConnection::readFromBuffer(Packet& packet)
{
    return buffer_.read(packet);
}
