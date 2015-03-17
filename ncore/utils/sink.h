#ifndef NCORE_UTILS_SINK_H_
#define NCORE_UTILS_SINK_H_

#include <ncore/ncore.h>
#include <ncore/sys/spin_lock.h>
#include <ncore/sys/mutex.h>
#include <ncore/algorithm/cityhash.h>



namespace ncore
{


class Sink
{
public:

    typedef uint32_t SlotId;

    /*wrap around google's cityhash*/
    template<typename SignalType>
    static SlotId GenSlotId()
    {
        static SlotId hash = 0;

        if (hash == 0)
        {
            const char * name = typeid(SignalType).name();
            int length = strlen(name);
            hash = static_cast<SlotId>(CityHash64(name, length));
        }
        return hash;
    }

    class SignalBase
    {
    public:
        SignalBase(){};
        virtual ~SignalBase(){};
        virtual SlotId GetSlotId() const = 0;
    };

    class SlotListener
    {
        friend class Sink;
    protected:
        virtual void OnNotify(const SignalBase * base) = 0;
        virtual SlotId GetSlotId() = 0; 
    };

    template<typename SignalType>
    class SlotListenerImp : public SlotListener
    {
    protected:
        void OnNotify(const SignalBase * base)
        {
            typedef SignalImp<SignalType> ImpType;

            auto signal = static_cast<const ImpType*>(base);
            if(signal)
                OnNotify(signal->GetPayload());
        }

        SlotId GetSlotId()
        {
            return GenSlotId<SignalType>();
        }

        virtual void OnNotify(const SignalType & signal) = 0;
    };

    template<typename T, typename SignalType>
    class SlotListenerAdapter : public SlotListenerImp<SignalType> 
    {
    private:
        typedef void (T::*FT)(const SignalType & param);

    public:
        SlotListenerAdapter()
        {
            obj_ = 0;
            fun_ = 0;
        };

        ~SlotListenerAdapter(){};

        void Register(T * obj, FT fun)
        {
            obj_ = obj;
            fun_ = fun;
        }

        void Unregister()
        {
            obj_ = 0;
            fun_ = 0;
        }

    protected:
        void OnNotify(const SignalType & signal)
        {
            if(obj_ && fun_)
                (obj_->*fun_)(signal);
        }

    private:
        T * obj_;
        FT fun_;
    };

    typedef std::shared_ptr<SignalBase> SignalBasePtr;

    template<typename SignalType>
    static SignalBasePtr Wrap(const SignalType & payload)
    {
        auto signal = new SignalImp<SignalType>();
        signal->SetPayload(payload);
        return SignalBasePtr(signal);
    }

private:
    template<typename SignalType>
    class SignalImp : public SignalBase
    {
    public:
        SignalImp() {}

        ~SignalImp() {}

        SlotId GetSlotId() const 
        { 
            return GenSlotId<SignalType>();
        }

        void SetPayload(const SignalType & payload)
        {
            payload_ = payload;
        }

        const SignalType & GetPayload() const
        {
            return payload_;
        }

    private:
        SignalType payload_;
    };
    
public:
    Sink();
    ~Sink();

    bool Register(SlotListener * listener);
    void Unregister(SlotListener * listener);

    void Shake();

    void Pour(SignalBasePtr & signal);

    void Emit(SignalBasePtr & signal);

    void Emit(SignalBase & signal);

private:
    typedef std::set<SlotListener *> ListenerSet;
    typedef std::map<SlotId, ListenerSet> SlotMap;
    typedef std::queue<SignalBasePtr> SignalQueue;

    SignalQueue signals_;
    SlotMap slots_;
    CriticalSection mutex_;
};


}

#endif
