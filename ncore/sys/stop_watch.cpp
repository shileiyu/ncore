#include "stop_watch.h"

namespace ncore
{

StopWatch::StopWatch()
{
    start_tick_ = 0;
    stop_tick_ = 0;
    stoped_ = true;
}

void StopWatch::Start()
{
    start_tick_ = ::GetTickCount();
    stoped_ = false;
}

void StopWatch::Stop()
{
    stop_tick_ = ::GetTickCount();
    stoped_ = true;
}

uint32_t StopWatch::ElapsedTime() const
{
    if(stoped_)
        return stop_tick_ - start_tick_;
    else
        return ::GetTickCount() - start_tick_;
}

}