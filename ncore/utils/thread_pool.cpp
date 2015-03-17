#include <ncore/sys/wait.h>
#include "thread_pool.h"

namespace ncore
{


ThreadPool::ThreadPool()
    : work_threads_(), running_(0)
{
    exec_proc_.Register(this, &ThreadPool::DoJobs);
}

ThreadPool::~ThreadPool()
{
    fini();
}

bool ThreadPool::init(size_t thread_number)
{
    if(!thread_number)
        return false;

    if(!job_queue_semaphore_.init(0, 512))
        return false;

    work_threads_.resize(thread_number);
    for(size_t i = 0; i < thread_number; ++i)
    {
        ThreadPtr thread(new Thread());

        if(thread == 0)
            return false;

        if(thread->init(exec_proc_) == false)
            return false;

        work_threads_[i] = std::move(thread);
    }

    return true;
}

void ThreadPool::fini()
{
    for(auto thread = work_threads_.begin(); 
        thread != work_threads_.end(); 
        ++thread)
    {
        (*thread)->Abort();
    }
    work_threads_.clear();

    while(!job_queue_.empty())
    {
        JobPtr job = job_queue_.front();
        job->Cancel();
        job->completed_.Set();
        job_queue_.pop();
    }
}

bool ThreadPool::Start()
{
    for(auto thread = work_threads_.begin(); 
        thread != work_threads_.end(); 
        ++thread)
    {
        if(!(*thread)->Start())
            return false;
    }
    return true;
}

void ThreadPool::Abort()
{
    for(auto thread = work_threads_.begin(); 
        thread != work_threads_.end(); 
        ++thread)
    {
        (*thread)->Abort();
    }
}

bool ThreadPool::Join()
{
    for(auto thread = work_threads_.begin(); 
        thread != work_threads_.end(); 
        ++thread)
    {
        if(!(*thread)->Join())
            return false;
    }
    return true;
}

void ThreadPool::QueueJob(JobPtr & ptr)
{
    if(ptr == nullptr)
        return;

    if(!ptr->ready_)
        return;

    if(!ptr->completed_.Wait(0))
        return;

    if(!ptr->completed_.Reset())
        return;

    job_queue_lock_.Acquire();
    job_queue_.push(ptr);
    job_queue_lock_.Release();
    job_queue_semaphore_.Increase(1);
}

void ThreadPool::DoJobs()
{
    try
    {
        while(true)
        {
            JobPtr job(nullptr);

            job_queue_semaphore_.WaitEx(Wait::kInfinity, true);
            job_queue_lock_.Acquire();
            job = job_queue_.front();
            job_queue_.pop();
            job_queue_lock_.Release();

            if(job == nullptr)
                continue;

            ++running_;
            if(!job->Rest(0))
                job->Do();
            job->completed_.Set();
            --running_;
        }
    }
    catch (ThreadExceptionAbort e)
    {
        return;
    }
}


}