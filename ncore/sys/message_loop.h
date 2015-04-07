#ifndef NCORE_SYS_MESSAEG_LOOP_H_
#define NCORE_SYS_MESSAEG_LOOP_H_

#include <ncore/ncore.h>

namespace ncore
{

class MessageLoop
{
    friend class MessageLoopRegistry;
public:
    static MessageLoop * Current();

public:
    class Observer
    {
        friend class MessageLoop;
    protected:
        virtual uint32_t OnIdle(MessageLoop & loop) = 0;
    };

public:
    int Run();
    int Run(bool alertable);
    int Run(bool alertable, const bool & stop_signal);
    void Exit(int code);
    //make queue empty
    void Purge();

    void AddObserver(Observer * observer);
    void RemoveObserver(Observer * observer);
    uint32_t Depth() const;
    uint32_t IdleTick() const;
    void SetTimeout(uint32_t timeout);
private:
    MessageLoop();
    ~MessageLoop();
    void SetThreadHandle(void * handle);
    bool Wait(uint32_t time_out, bool alertable);
    bool IsExitting(uint32_t & exit_code);
    void RunOnce();
    void OnIdle();
private:
    uint32_t depth_;
    uint32_t idle_tick_;
    uint32_t timeout_;
    std::set<Observer*> observers_;
};

}

#endif