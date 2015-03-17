#include "timer.h"

namespace ncore
{
     
class TimerRoutine
{

private:
    static void CALLBACK APCRoutine(LPVOID lpArg, 
                                    DWORD dwTimerLowValue,
                                    DWORD dwTimerHighValue)
    {
        assert(lpArg);
        Timer * timer = reinterpret_cast<Timer *>(lpArg);
        timer->OnTime();
        return;
    }

    friend class Timer;
};
        
Timer::Timer()
    : timer_handle_(0),
      interval_(0),
      timer_proc_(0)
{

}

Timer::~Timer()
{
    fini();
}

bool Timer::init(TimerProc & timer_proc)
{
    fini();

    timer_handle_ = CreateWaitableTimer(0, false, 0);
    if(timer_handle_ == 0)
        return false;

    timer_proc_ = &timer_proc;
    return true;
}

void Timer::fini()
{
    if (timer_handle_)
    {
        CloseHandle(timer_handle_);
        timer_handle_ = 0;
    }
    timer_proc_ = 0;
}

bool Timer::Start()
{
    if (timer_handle_ == 0)
        return false;

    LARGE_INTEGER due_time;
    due_time.QuadPart = -10000;
    due_time.QuadPart *= interval_;

    BOOL result = SetWaitableTimer(timer_handle_, &due_time, interval_, 
                                   TimerRoutine::APCRoutine, this, false);

    if(!result)
        return false;

    return true;
}

void Timer::Stop()
{
    if(timer_handle_)
    {
        CancelWaitableTimer(timer_handle_);
    }
}

void Timer::SetInterval(uint32_t ms)
{
    interval_ = ms;
}

void Timer::OnTime()
{
    if (timer_proc_)
        timer_proc_->OnTime();
    else
        Stop();
}


}