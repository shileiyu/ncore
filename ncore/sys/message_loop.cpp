#include "message_loop.h"

namespace ncore
{

MessageLoop * MessageLoop::Current()
{
    const size_t kTopStack = 4;
    MessageLoop * cur_loop = 0;
#ifdef NCORE_X86
    auto stack_top = reinterpret_cast<void **>(__readfsdword(4));
    if(stack_top)
    {
        cur_loop = reinterpret_cast<MessageLoop *>(*(--stack_top));
    }
#endif
    return cur_loop;
}

void MessageLoop::SetCurrent(MessageLoop * loop)
{
    const size_t kTopStack = 4;
    MessageLoop * cur_loop = 0;
#ifdef NCORE_X86
    auto stack_top = reinterpret_cast<void **>(__readfsdword(4));
    if(stack_top)
    {
        *(--stack_top) = loop;
    }
#endif
    return;
}

MessageLoop::MessageLoop() 
    : depth_(0), idle_tick_(0), timeout_(15), dummy_handle_(0)
{
    dummy_handle_ = CreateEvent(0, FALSE, FALSE, 0);
}

MessageLoop::~MessageLoop()
{
    if(dummy_handle_)
    {
        ::CloseHandle(dummy_handle_);
        dummy_handle_ = 0;
    }
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

void MessageLoop::ForceCreate()
{
    auto loop = Current();
    if(!loop)
    {
        SetCurrent(this);
    }
    MSG msg = {0};
    ::PeekMessage(&msg, 0, WM_USER, WM_USER, PM_NOREMOVE);
    return;
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

    if(dummy_handle_ == 0)
        return 0xBADBADBA;

    if(depth_ == 0)
    {
        SetCurrent(this);
    }

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
        //clean magic pointer
        SetCurrent(0);
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
    DWORD wait_flag = MWMO_INPUTAVAILABLE;
    if(alertable)
        wait_flag |= MWMO_ALERTABLE;

    DWORD wait_result = 0;
    wait_result = MsgWaitForMultipleObjectsEx(1, &dummy_handle_, time_out,
                                              QS_ALLINPUT, wait_flag);
    switch(wait_result)
    {
    case WAIT_TIMEOUT:
        return false;
    case WAIT_IO_COMPLETION:
    case WAIT_OBJECT_0 + 1:
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