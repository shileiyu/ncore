#ifndef NCORE_SYS_BACkGROUND_THREAD
#define NCORE_SYS_BACkGROUND_THREAD

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include "proactor.h"
#include "thread.h"

namespace ncore
{


class BackgroundThread : public NonCopyableObject
{
public:
    static bool Initialize();
    static void Uninitialize();
    static Proactor & IOHandler();
private:
    static BackgroundThread instance_;
private:
    BackgroundThread();
    ~BackgroundThread();

    bool Startup();
    void Shutdown();
    void IOProc();

private:
    Proactor io_handler_;
    Thread io_thread_;
    ThreadProcAdapter<BackgroundThread> io_proc_;

    friend class BGIOThreadProc;
};


}

#endif