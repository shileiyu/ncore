#include "sink.h"

namespace ncore
{

Sink::Sink()
{
    mutex_.init();
};

Sink::~Sink()
{
    mutex_.fini();
};

bool Sink::Register(SlotListener * listener)
{
    if(listener == 0)
        return false;

    SlotId slot_id = listener->GetSlotId();
    ListenerSet & slot = slots_[slot_id];
    slot.insert(listener);
    return true;
}

void Sink::Unregister(SlotListener * listener)
{
    if(listener == 0)
        return;

    SlotId slot_id = listener->GetSlotId();
    ListenerSet & slot = slots_[slot_id];
    slot.erase(listener);
    return;
}

void Sink::Shake()
{
    mutex_.Enter();
    while(!signals_.empty())
    {
        auto signal = signals_.front();
        signals_.pop();

        if(signal == 0)
            continue;

        SlotId slot_id = signal->GetSlotId();
        ListenerSet & slot = slots_[slot_id];

        for(auto iter = slot.begin(); iter != slot.end();)
        {
            auto listener = (*iter);
            ++iter;
            mutex_.Leave();
            listener->OnNotify(signal.get());
            mutex_.Enter();
        }
    }
    mutex_.Leave();
}

void Sink::Pour(SignalBasePtr & signal)
{
    ScopedCriticalSection scope_mutex(&mutex_); 
    if(signal)
        signals_.push(signal);
}

void Sink::Emit(SignalBasePtr & signal)
{
    if(!signal)
        return;

    mutex_.Enter();
    SlotId slot_id = signal->GetSlotId();
    ListenerSet & slot = slots_[slot_id];
    for(auto iter = slot.begin(); iter != slot.end();)
    {
        auto listener = (*iter);
        ++iter;
        mutex_.Leave();
        listener->OnNotify(signal.get());
        mutex_.Enter();
    }
    mutex_.Leave();
}

void Sink::Emit(SignalBase & signal)
{
    mutex_.Enter();
    SlotId slot_id = signal.GetSlotId();
    ListenerSet & slot = slots_[slot_id];
    for(auto iter = slot.begin(); iter != slot.end();)
    {
        auto listener = (*iter);
        ++iter;
        mutex_.Leave();
        listener->OnNotify(&signal);
        mutex_.Enter();
    }
    mutex_.Leave();
}

}
