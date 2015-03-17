#include "background_thread.h"

namespace ncore
{


BackgroundThread BackgroundThread::instance_;


bool BackgroundThread::Initialize()
{
    return instance_.Startup();
}

void BackgroundThread::Uninitialize()
{
    instance_.Shutdown();
}

BackgroundThread::BackgroundThread()
{
}

BackgroundThread::~BackgroundThread()
{
    Shutdown();
}

Proactor & BackgroundThread::IOHandler()
{
    return instance_.io_handler_;
}

bool BackgroundThread::Startup()
{
    io_proc_.Register(this, &BackgroundThread::IOProc);

    if( io_handler_.init() &&
        io_thread_.init(io_proc_))
    {
        return io_thread_.Start();
    }

    return false;
}

void BackgroundThread::Shutdown()
{
    io_thread_.Abort();
    io_thread_.fini();
    io_handler_.fini();
}

void BackgroundThread::IOProc()
{
    while(true)
    {
        io_handler_.Run(100);
    }
}


}