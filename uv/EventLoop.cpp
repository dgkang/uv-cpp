/*
   Copyright 2017, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net
    
   Last modified: 2019-6-17
    
   Description: https://github.com/wlgq2/libuv_cpp11
*/

#include "EventLoop.h"
#include "TcpConnection.h"
#include "Async.h"

using namespace uv;

EventLoop::EventLoop()
    :EventLoop(EventLoop::Mode::New)
{
}

EventLoop::EventLoop(EventLoop::Mode mode)
    : isRun_(false),
    loop_(nullptr),
    async_(nullptr)
{
    if (mode == EventLoop::Mode::New)
    {
        loop_ = new uv_loop_t();
        ::uv_loop_init(loop_);
    }
    else
    {
        loop_ = uv_default_loop();
    }
    async_ = new Async(this);
}

EventLoop::~EventLoop()
{
    if (loop_ != uv_default_loop())
    {
        delete async_;
        uv_loop_close(loop_);
        delete loop_;
    }
}

EventLoop* uv::EventLoop::DefalutLoop()
{
    static EventLoop* defaultLoop = new uv::EventLoop(uv::EventLoop::Mode::Default);
    return defaultLoop;
}

uv_loop_t* EventLoop::hanlde()
{
    return loop_;
}

int EventLoop::run()
{
    loopThreadId_ = std::this_thread::get_id();
    isRun_ = true;
    return ::uv_run(loop_, UV_RUN_DEFAULT);
}

int uv::EventLoop::runNoWait()
{
    loopThreadId_ = std::this_thread::get_id();
    isRun_ = true;
    return ::uv_run(loop_, UV_RUN_NOWAIT);
}


bool EventLoop::isRunInLoopThread()
{
    if (isRun_)
    {
        return std::this_thread::get_id() == loopThreadId_;
    }
    //EventLoopδ����.
    return false;
}

void uv::EventLoop::runInThisLoop(const DefaultCallback func)
{
    if (nullptr == func)
        return;

    if (isRunInLoopThread())
    {
        func();
        return;
    }
    async_->runInThisLoop(func);
}

const char* EventLoop::GetErrorMessage(int status)
{
    if (WriteInfo::Disconnected == status)
    {
        static char info[] = "the connection is disconnected";
        return info;
    }
    return uv_strerror(status);
}