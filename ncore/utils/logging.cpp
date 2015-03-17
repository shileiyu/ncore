#include <ncore/sys/application.h>
#include <ncore/sys/path.h>
#include <ncore/sys/file_stream.h>
#include <ncore/encoding/utf8.h>
#include "karma.h"
#include "scope_handle.h"
#include "logging.h"

namespace ncore
{

SpinLock Log::locker;
LogDestination Log::current_dest = kLogToBlackhole;
LogLevel Log::current_level = kLogError;
char Log::logfile[kMaxPath8] = {"bianque.log"};
char Log::dumper[kMaxPath8] = {"dumper.exe"};

static const char * kLevelName[kLogLevelCount] = 
{ "[Verbose]", "[Info]", "[Warning]", "[Error]", "[Fault]" };


class LogRoutines
{
private:
    friend class Log;

    static LONG WINAPI CallDumper(EXCEPTION_POINTERS * ep)
    {

        Log::locker.Acquire();
        CallDumperInternal(ep);
        Log::locker.Release();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    static void CallDumperInternal(EXCEPTION_POINTERS * ep)
    {
        //call dumper
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi ={0};
        ScopeHandle<Win32GeneralHandle> real_self = 0;
        HANDLE self = GetCurrentProcess();
        DWORD pid = GetCurrentProcessId();
        DWORD tid = GetCurrentThreadId();
        DWORD desire_access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
                              PROCESS_DUP_HANDLE;

        si.cb = sizeof(si);
        if(!::DuplicateHandle(self, self, self, &real_self, 
                            desire_access, TRUE, 0))
        {
            return;
        }

        wchar_t cmd[0x200] = {0};

        Karma::Format(cmd, L"--parent:%u --pid:%u --tid:%u --ep:%u",
                      *&real_self, pid, tid, ep);

        wchar_t dumper16[kMaxPath16] = {0};

        if(!::CreateProcess(dumper16, cmd, 0, 0, TRUE, 0, 0, 0, &si, &pi))
            return;

        ::WaitForSingleObject(pi.hProcess, INFINITE);

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
        return;
    }

    static void DebugBreak()
    {
        ::DebugBreak();
    }

};


void Log::EnableCrashDump()
{
    SetUnhandledExceptionFilter(LogRoutines::CallDumper);
}

void Log::AlterLogFile(const char * filename)
{
    if(strlen(filename) >= kMaxPath8)
        return;
    Karma::Copy(logfile, filename);
}

void Log::AlterLogDestination(LogDestination dest)
{
    current_dest = dest;
}

void Log::AlterLogLevel(LogLevel lv)
{
    current_level = lv;
}

void Log::AlterDumper(const char * filename)
{
    if(strlen(filename) >= kMaxPath8)
        return;
    Karma::Copy(dumper, filename);
}

Log::Log(const char * file, int line, LogLevel level)
    :file_(file),
     line_(line),
     level_(level),
     lazy_stream_(0)
{
    if(level_ >= current_level)
    {
        InitializeStream();
    }
}

Log::~Log()
{
    AppendTail();
    WriteToDestination();
    UninitializeStream();
    DoAbortIfFault();
}

void Log::InitializeStream()
{
    if(lazy_stream_ == 0)
    {
        lazy_stream_ = new std::ostringstream();
        if(lazy_stream_)
        {
            auto & out = *lazy_stream_;

            if(level_ >= kLogVerbose && level_ < kLogLevelCount)
                out << kLevelName[level_] << ": ";
            else
                out << "[Level_" << level_ << "]: ";
        }
    }
}

void Log::UninitializeStream()
{
    if(lazy_stream_)
    {
        delete lazy_stream_;
        lazy_stream_ = 0;
    }
}

void Log::AppendTail()
{
    if(lazy_stream_)
    {
        (*lazy_stream_) << " " << file_ << " " << line_ << std::endl;
        lazy_stream_->flush();
    }
}

void Log::WriteToDestination()
{
    WriteToFile();
    WriteToDebugView();
}

void Log::WriteToFile()
{

    if(lazy_stream_ == 0)
        return;

    if(!(current_dest & kLogToFile))
        return;

    DWORD btw = 0;
    std::string & content = lazy_stream_->str();

    //if logfile contain base name only. We don't need to create the directory.
    auto log_file_path = Path::GetPathFromFullName(logfile);
    if(!log_file_path.empty())
        Path::CreateDirectoryRecursive(log_file_path.data());

    FileStream log;
    if(log.init(logfile, FileAccess::kWrite, FileShare::kShareRead, 
                FileMode::kOpenOrCreate, FileAttribute::kNormal, 
                FileOption::kWriteThrough))
    {
        int64_t zero = 0;
        uint32_t transfered = 0;

        locker.Acquire();
        log.Seek(zero, FilePosition::kEnd);
        log.Write(content.data(), content.length(), transfered);
        locker.Release();
    }

    return;
}

void Log::WriteToDebugView()
{
    if(lazy_stream_ == 0)
        return;

    if(!(current_dest & kLogToDebugView))
        return;

    ::OutputDebugStringA(lazy_stream_->str().data());
}

void Log::WriteToStdOutput()
{
    if(lazy_stream_ == 0)
        return;
    printf(lazy_stream_->str().data());
}

void Log::DoAbortIfFault()
{
    if(level_ == kLogFault)
    {
        LogRoutines::DebugBreak();
        Application::Exit(0xBADBADBA);
    }
}

Log::operator bool() const
{
    return true;
}


}