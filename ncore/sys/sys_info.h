#ifndef NCORE_SYS_SYS_INFO_H_
#define NCORE_SYS_SYS_INFO_H_

#include <ncore/ncore.h>

namespace ncore
{

class SysInfo
{
public:
    static int GetLogicalProcessorNumber();

    static bool DWMEnabled();

    static bool IsWow64();

    static bool QueryDiskFreeSpace(const char * dir, 
                                   uint64_t & freebytes);

    static uint32_t GetMaxFreeSpaceDisk();

    static uint32_t TickCount();

    static std::string CommonAppDataPath();

    static std::string CommonAppDataPath(const char * name);

    static std::string WindowsPath();

    static std::string WindowsPath(const char * name);

    static std::string GetPlatformName();

    static bool IsElevated();

    static bool IsUserAdmin();

    static bool IsWinNT5();

    static bool IsWinXP();

    static bool IsWin2K3();

    static bool IsWinNT6();

    static bool IsX86();

    static bool IsX64();
};

}
#endif