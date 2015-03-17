#ifndef NCORE_UTILS_THREAD_POOL_H_
#define NCORE_UTILS_THREAD_POOL_H_


#include <ncore/ncore.h>
#include <ncore/base/atomic.h>
#include <ncore/sys/spin_lock.h>
#include <ncore/sys/thread.h>
#include "job.h"

namespace ncore
{

class Thread;
class ThreadProc;

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    bool init(size_t thread_number);
    void fini();

    bool Start();
    void Abort();
    bool Join();

    void QueueJob(JobPtr & ref);

private:
    void DoJobs();

private:
    Atomic running_;
    SpinLock job_queue_lock_;
    Semaphore job_queue_semaphore_;
    ThreadProcAdapter<ThreadPool> exec_proc_;
    std::queue<JobPtr> job_queue_;

    typedef std::unique_ptr<Thread> ThreadPtr;
    std::vector<ThreadPtr> work_threads_;
};


}

#endif