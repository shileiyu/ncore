#include "thread.h"


namespace ncore
{

__declspec(thread) Thread * current = 0;

class MainThread
{
public:
    MainThread() 
    {
        thread_.thread_handle_ = (uintptr_t)GetCurrentThread();
        thread_.thread_id_ = GetCurrentThreadId();
        thread_.started_ = true;
        current = &thread_;

    }

    ~MainThread()
    {
        //avoid stuck in Thread::fini().
        thread_.thread_handle_ = 0;
    }

    Thread thread_;
};

MainThread main;



uint32_t _stdcall Thread::Run(void * param)
{
    Thread * thread = (Thread *)param;
    assert(thread != 0);
    try
    {
        thread->started_ = true;
        assert(thread->thread_proc_ != 0);
        thread->thread_proc_->Run();
    }
    catch(ThreadExceptionAbort e)
    {
        return kThreadAbort;
    }
    return kThreadExited;
}

VOID  _stdcall Thread::AbortProc(ULONG_PTR dwParam)
{
    Thread * thread = (Thread *)dwParam;
    if(thread->started_)
        throw ThreadExceptionAbort();
    ::ExitThread(kThreadAbort);
}

void Thread::Sleep(int ms,bool alertable)
{
    SleepEx(ms,alertable);
}

uint32_t Thread::GetCurrentThreadId()
{
    return ::GetCurrentThreadId();
}

Thread * Thread::Current()
{
    return current;
}

Thread * Thread::Main()
{
    return &main.thread_;
}

Thread::Thread()
    : thread_proc_(0),
      thread_handle_(0),
      thread_id_(0),
      started_(false)
{
}

Thread::~Thread()
{
    fini();
}

bool Thread::init(ThreadProc & thread_proc)
{
    if(thread_handle_)
        return true;

    thread_proc_ = &thread_proc;

    thread_handle_ = _beginthreadex(0, 0, Run, this, CREATE_SUSPENDED, 
                                    &thread_id_);

    if(thread_handle_ == 0)
        return false;

    current = this;
    return true;
}

void Thread::fini()
{
    if(thread_handle_ != 0)
    {
        Join();
        CloseHandle((HANDLE)thread_handle_);
        thread_handle_ = 0;
    }
    thread_id_ = 0;
    thread_proc_ = 0;
    current = 0;
}

bool Thread::Start()
{
    if(thread_handle_)
    {
        HANDLE handle = (HANDLE)thread_handle_;
        return ::ResumeThread(handle) == 1;
    }
    return false;
}

bool Thread::Terminate()
{
    if(thread_handle_)
    {
        HANDLE handle = (HANDLE)thread_handle_;
        return ::TerminateThread(handle, kTheadTerminated) != 0;
    }
    return false;
}

bool Thread::Suspend()
{
    if(thread_handle_)
    {
        HANDLE handle = (HANDLE)thread_handle_;
        return ::SuspendThread(handle) > 0;
    }
    return false;
}

bool Thread::Join()
{
    if(thread_handle_)
    {
        HANDLE handle = (HANDLE)thread_handle_;
        return ::WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
    }
    return false;
}

bool Thread::Abort()
{
    if(thread_handle_)
    {
        HANDLE handle = (HANDLE)thread_handle_;
        ULONG_PTR param = (ULONG_PTR)this;
        return QueueUserAPC(AbortProc, handle, param) != 0;
    }
    return false;
}

bool Thread::IsAlive()
{
    if(thread_handle_)
    {
        HANDLE handle = (HANDLE)thread_handle_;
        return WaitForSingleObject(handle, 0) == WAIT_TIMEOUT;
    }
    return false;
}


}