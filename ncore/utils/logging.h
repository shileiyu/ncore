#ifndef NCORE_UTILS_LOGGING_H_
#define NCORE_UTILS_LOGGING_H_

#include <ncore/ncore.h>
#include <ncore/sys/spin_lock.h>

namespace ncore
{

enum LogLevel
{
    kLogVerbose,        //everything
    kLogInfo,           //useful message.
    kLogWarning,        //recoverable error.
    kLogError,          //unrecoverable errir.
    kLogFault,          //critical error.
    kLogLevelCount,
};

enum LogDestination
{
    kLogToBlackhole,
    kLogToFile,
    kLogToDebugView,
    kLogToBoth,
};

//alway use LOG_xxx macro instead of use Log directly.
class Log
{
public:
    static void AlterLogFile(const char * filename);
    static void AlterLogDestination(LogDestination dest);
    static void AlterLogLevel(LogLevel lv);
    static void AlterDumper(const char * filename);
    static void EnableCrashDump();
private:
    static LogLevel current_level;
    static LogDestination current_dest;
    static char logfile[];
    static char dumper[];
    static SpinLock locker;
public:
    Log(const char * file, int line, LogLevel level);
    ~Log();

    template<typename T>
    Log & operator<<(const T & trait)
    {
        if(lazy_stream_)
            (*lazy_stream_) << trait;
        return *this;
    }

    Log & operator<<(std::ostream &(*new_line_feeder)(std::ostream &)) 
    {
        if(lazy_stream_)
            new_line_feeder(*lazy_stream_);
        return *this;
    }
    //make assert happy
    operator bool() const;

private:
    void InitializeStream();
    void UninitializeStream();
    void AppendTail();
    void WriteToDestination();
    void WriteToFile();
    void WriteToDebugView();
    void WriteToStdOutput();
    void DoAbortIfFault();

private:
    LogLevel level_;
    const char * file_;
    int line_;
    std::ostringstream * lazy_stream_;

    friend class LogRoutines;
};

}

#define LOG_VERBOSE ncore::Log(__FILE__, __LINE__, ncore::kLogVerbose)
#define LOG_INFO    ncore::Log(__FILE__, __LINE__, ncore::kLogInfo)
#define LOG_WARNING ncore::Log(__FILE__, __LINE__, ncore::kLogWarning)
#define LOG_ERROR   ncore::Log(__FILE__, __LINE__, ncore::kLogError)
#define LOG_FAULT   ncore::Log(__FILE__, __LINE__, ncore::kLogFault)

#endif