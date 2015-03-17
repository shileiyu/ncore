#ifndef NSSSHARE_UTILS_PERIOD_H_
#define NSSSHARE_UTILS_PERIOD_H_

#include <ncore/ncore.h>
#include <ncore/sys/sys_info.h>

namespace ncore
{


template<typename Object>
class Period
{
    typedef void (Object::*Func)();
public:
    Period() 
        : obj_(0), func_(0), 
          due_(0), interval_(0),
          active_(false)
    {
    }

    void set_internal(uint32_t ms)
    {
        interval_ = ms;
    }

    void Register(Object * obj, Func func)
    {
        obj_ = obj;
        func_ = func;
    }

    void Run()
    {
        if(!active_)
            return;

        uint32_t tick = SysInfo::TickCount();
        if(tick - due_ > interval_)
        {
            if(obj_ && func_)
                (obj_->*func_)();
            due_ = tick;
        }
    }

    void set_active(bool val)
    {
        active_ = val;
    }

private:
    Object * obj_;
    Func func_;
    
    uint32_t due_;
    uint32_t interval_;
    bool active_;
    
};


}

#endif