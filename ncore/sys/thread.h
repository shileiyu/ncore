#ifndef NCORE_SYS_THREAD_H_
#define NCORE_SYS_THREAD_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include <ncore/base/exception.h>

namespace ncore
{

//线程过程类
class ThreadProc
{
protected:
    ThreadProc(){}
    virtual ~ThreadProc(){}
public:
    virtual void Run() = 0;
};

template<typename T>
class ThreadProcAdapter : public ThreadProc
{
public:
    typedef void (T::*TF)();
public:
    ThreadProcAdapter()
        :obj_(0),
        func_(0)
    {
    }

    ~ThreadProcAdapter()
    {
    }

    void Register(T * obj, TF func)
    {
        obj_ = obj;
        func_ = func;
    }

    void Run()
    {
        (obj_->*func_)();
    }

private:
    T * obj_;
    TF func_;
};

//线程异常

class ThreadExceptionAbort : public Exception
{
};


//线程类
class Thread : public NonCopyableObject
{
private:
    enum ThreadExitCode
    {
        kThreadAbort = -3,
        kTheadTerminated = -2,
        kThreadNotStarted = -1,
        kThreadExited = 0,
        kThreadRunning = 1,
    };

private:
#if defined NCORE_WINDOWS
    static uint32_t _stdcall Run(void *);
    static void  _stdcall AbortProc(ULONG_PTR dwParam);
#elif defined NCORE_MACOS

#endif
public:
    static uint32_t GetCurrentThreadId();
    static void Sleep(int ms, bool alertable);
public:
    Thread();
    virtual ~Thread();
    bool init(ThreadProc & thread_proc);
    void fini();

    bool Start();
    bool Terminate();
    bool Suspend();
    bool Join();
    bool IsAlive();
    bool Abort();

protected:
    uintptr_t thread_handle_;
    uint32_t thread_id_;
    ThreadProc * thread_proc_;
    bool started_;
};


}

#endif