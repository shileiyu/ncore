#ifndef NCORE_SYS_TIMER_H
#define NCORE_SYS_TIMER_H

#include <ncore/ncore.h>

namespace ncore
{

class TimerProc
{
public:
    virtual void OnTime() = 0;
};

template<typename T>
class TimerProcAdapter
    : public TimerProc
{
private:
    typedef void (T::*FT)();

public:
    TimerProcAdapter()
        : obj_(0),
        func_(0)
    {

    }

    ~TimerProcAdapter()
    {

    }

    virtual void OnTime() override
    {
        (obj_->*func_)();
    }

    void Register(T * obj, FT func)
    {
        obj_ = obj;
        func_ = func;
    }

private:
    T * obj_;
    FT func_;
};

class Timer
{
public:
    Timer();
    ~Timer();

    bool init(TimerProc & timer_proc);
    void fini();

    bool Start();
    void Stop();

    void SetInterval(uint32_t ms);

private:
    void OnTime();

private:
    void * timer_handle_;
    uint32_t interval_;
    TimerProc * timer_proc_;

    friend class TimerRoutine;
};


}
#endif