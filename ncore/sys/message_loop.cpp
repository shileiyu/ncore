#include "spin_lock.h"
#include "message_loop.h"

namespace ncore
{

class MessageLoopRegistry
{
public:
    MessageLoopRegistry() {}

    ~MessageLoopRegistry() 
    {
        for(auto item = loops_.begin(); item != loops_.end(); ++item) 
        {
            MessageLoop * loop = *item;
            delete loop;
        }
        loops_.clear();
    }
    void Register(MessageLoop * loop) 
    {
        lock_.Acquire();
        loops_.insert(loop);
        lock_.Release();
    }
private:
    SpinLock lock_;
    std::set<MessageLoop *> loops_;
};

MessageLoopRegistry registry;
__declspec(thread) MessageLoop * current = nullptr;

MessageLoop * MessageLoop::Current()
{
    if(current == nullptr)
        if(current = new MessageLoop())
            registry.Register(current);
    return current;
}

MessageLoop::MessageLoop() 
    : depth_(0), idle_tick_(0), timeout_(15)
{
}

MessageLoop::~MessageLoop()
{
    observers_.clear();
}

int MessageLoop::Run()
{
    return Run(false);
}

int MessageLoop::Run(bool alertable)
{
    bool stop = false;
    return Run(alertable, stop);
}

void MessageLoop::Exit(int code)
{
    ::PostQuitMessage(code);
}

void MessageLoop::Purge()
{
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        DispatchMessage(&msg); 
    }
}

void MessageLoop::AddObserver(MessageLoop::Observer * observer)
{
    observers_.insert(observer);
}

void MessageLoop::RemoveObserver(MessageLoop::Observer * observer)
{
    observers_.erase(observer);
}

int MessageLoop::Run(bool alertable, const bool & stop_signal)
{
    uint32_t exit_code = 0;
    bool condition = true;

    ++depth_;

    do
    {    
        if(Wait(timeout_, alertable))
        {
            idle_tick_ = 0;
            if(stop_signal || IsExitting(exit_code))
                condition = false;
            else
                RunOnce();
        }
        else {
            ++idle_tick_;
        }
        OnIdle();
    }
    while(condition);

    --depth_;

    if(depth_ == 0)
    {
        //clean message queue
        Purge();
    }

    return exit_code;
}

uint32_t MessageLoop::Depth() const
{
    return depth_;
}

uint32_t MessageLoop::IdleTick() const
{
    return idle_tick_;
}

void MessageLoop::SetTimeout(uint32_t timeout)
{
    timeout_ = timeout;
}

bool MessageLoop::Wait(uint32_t time_out, bool alertable)
{
    DWORD wait_flag = MWMO_INPUTAVAILABLE | MWMO_WAITALL;
    if(alertable)
        wait_flag |= MWMO_ALERTABLE;

    DWORD wait_result = 0;
    HANDLE dummy_handle = 0;
    wait_result = MsgWaitForMultipleObjectsEx(0, &dummy_handle, time_out,
                                              QS_ALLINPUT, wait_flag);
    switch(wait_result)
    {
    case WAIT_TIMEOUT:
        return false;
    case WAIT_IO_COMPLETION:
    case WAIT_OBJECT_0:
        return true;
    default:
        assert(0);
        return false;
    }
}

bool MessageLoop::IsExitting(uint32_t & exit_code)
{
    MSG msg;
    if(::PeekMessage(&msg, 0, WM_QUIT, WM_QUIT, PM_NOREMOVE))
    {
        exit_code = msg.wParam;
        return true;
    }
    return false;
}

void MessageLoop::RunOnce()
{
    MSG msg;

    if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if(msg.message != WM_QUIT)
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
        else
        {
            Exit(msg.wParam);
        }
    }
    return;
}

void MessageLoop::OnIdle()
{
    for (auto item = observers_.begin(); item != observers_.end();) {
        auto observer = *item;
        ++item;
        observer->OnIdle(*this);
    }
}


}