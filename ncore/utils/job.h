#ifndef NCORE_UTILS_JOB_H_
#define NCORE_UTILS_JOB_H_


#include <ncore/ncore.h>
#include <ncore/sys/named_event.h>
#include <ncore/sys/semaphore.h>
#include <ncore/sys/spin_lock.h>

namespace ncore
{

class Job
{
    friend class ThreadPool;
public:
    Job() 
    {
        ready_ = false;
        if (canceled_.init(true, false)) {
            if (completed_.init(true, true)){
                ready_ = true;
            }
        }
    }

    virtual ~Job() {}

    void Cancel()
    {
        canceled_.Set();
    }

    bool Wait(uint32_t time)
    {
        if(ready_)
            return completed_.Wait(time);
        return true;
    }

protected:

    virtual void Do() {}

    bool Rest(uint32_t time)
    {
        if(ready_)
            return canceled_.WaitEx(time, true);
        return false;
    }

protected:
    bool ready_;
private:
    NamedEvent canceled_;
    NamedEvent completed_;
};

typedef std::tr1::shared_ptr<Job> JobPtr;


}

#endif