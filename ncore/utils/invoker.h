#ifndef NCORE_UTILS_INVOKER_H_
#define NCORE_UTILS_INVOKER_H_

#include <ncore/base/object.h>
#include <ncore/sys/spin_lock.h>



namespace ncore
{

class AsyncInvoker : public NonCopyableObject
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {}
        virtual void Invoke() = 0;
    };

    template<typename R>
    struct Future
    {
        R value;
        bool completed;

        template<typename T>
        Future<R> & operator<<(T & cb)
        {
            value = cb();
            completed = true;
            return *this;
        }

        R Value()
        {
            return value;
        }
    };

    template<>
    struct Future<void>
    {
        bool completed;

        template<typename T>
        Future<void> & operator<<(T & cb)
        {
            cb();
            completed = true;
            return *this;
        }

        void Value() {}
    };

    template<typename CB> 
    class Task : public Callback
    {
    public:
        typedef typename CB::result_type R;
        typedef std::shared_ptr<Task<CB>> Ptr;

        Task(const CB & cb) 
            : callback_(cb) {}

        void Invoke() 
        { 
            future_ << callback_;
        }

        R Result()
        {
            while(!future_.completed);
            return future_.Value();
        }

    private:
        CB callback_;
        Future<R> future_;  
    };

    typedef std::shared_ptr<Callback> CallbackPtr;

public:
    AsyncInvoker() {}

    bool ProcessOne()
    {
        CallbackPtr cb(0);
        queue_lock_.Acquire();
        if(!queue_.empty())
        {
            cb = queue_.front();
            queue_.pop();
        }
        queue_lock_.Release();

        if(!cb)
            return false;

        cb->Invoke();
        return true;
    }

    template<typename CB>
    typename Task<CB>::Ptr QueueTask(const CB & cb)
    {
        Task<CB>::Ptr task(new Task<CB>(cb));
        Enqueue(task);
        return task;
    }

    void Enqueue(const CallbackPtr & ptr)
    {
        queue_lock_.Acquire();
        queue_.push(ptr);
        queue_lock_.Release();
    }

private:
    SpinLock queue_lock_;
    std::queue<CallbackPtr> queue_;
};


}

#endif