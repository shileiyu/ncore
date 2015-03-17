#include <ncore/encoding/utf8.h>
#include "path.h"
#include "sys_info.h"

namespace ncore
{
#pragma warning(push)
#pragma warning(disable: 4996)
static bool QueryOSVersion(uint32_t & major, uint32_t & minor)
{
    OSVERSIONINFO info = { 0 };
    info.dwOSVersionInfoSize = sizeof(info);
    if (!GetVersionEx(&info))
        return false;
    major = info.dwMajorVersion;
    minor = info.dwMinorVersion;
    return true;
}
#pragma warning(pop)

int SysInfo::GetLogicalProcessorNumber()
{
    SYSTEM_INFO sys_info = {0};
    GetNativeSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors;
}

bool SysInfo::DWMEnabled()
{
    static const HMODULE dwm_moudle = LoadLibrary(L"dwmapi.dll");

    if(dwm_moudle)
    {
        BOOL value = TRUE;
        DwmIsCompositionEnabled(&value);
        return value==TRUE;
    }
    else
    {
        return false;
    }
}

bool SysInfo::IsWow64()
{
    //this code is copied from MSDN.
    typedef bool (WINAPI *IsWow64Process_T) (HANDLE, PBOOL);
    IsWow64Process_T TIsWow64Process = 0;
    BOOL wow64 = 0;
    TIsWow64Process = (IsWow64Process_T)::GetProcAddress(
        ::GetModuleHandle(L"kernel32"), "IsWow64Process");
    if (0 != TIsWow64Process)
    {
        if (!TIsWow64Process(::GetCurrentProcess(), &wow64))
            return false;
        else
            return true;
    }
    return TRUE == wow64 ;
}

bool SysInfo::QueryDiskFreeSpace(const char * path,
                                 uint64_t & freebytes)
{
    wchar_t path16[kMaxPath16];
    if(!UTF8::Decode(path, -1, path16, kMaxPath16))
        return false;

    ULARGE_INTEGER freespace = {0};
    if(::GetDiskFreeSpaceEx(path16, &freespace, 0, 0))
    {
        freebytes = freespace.QuadPart;
        return true;
    }
    return false;
}

uint32_t SysInfo::GetMaxFreeSpaceDisk()
{
    ULARGE_INTEGER freespace = {0};
    wchar_t dos_name[] = {L"A:"};
    uint64_t max_freebytes = 0;
    uint64_t curr_freebytes = 0;
    uint32_t max_disk_index = 0;
    DWORD bits = ::GetLogicalDrives();

    for(uint32_t i = 0; bits != 0; ++i, bits >>= 1)
    {
        dos_name[0] = 'A' + i;

        if((bits & 1) == 0)
            continue;
        if(GetDriveType(dos_name) != DRIVE_FIXED)
            continue;
        if(!::GetDiskFreeSpaceEx(dos_name, &freespace, 0, 0))
            continue;

        curr_freebytes = freespace.QuadPart;
        if(curr_freebytes > max_freebytes)
        {
            max_freebytes = curr_freebytes;
            max_disk_index = i;
        }
    }
    return max_disk_index;
}

uint32_t SysInfo::TickCount()
{
    return ::GetTickCount();
}

std::string SysInfo::CommonAppDataPath()
{
    wchar_t path16[kMaxPath16] = {0};
    char path[kMaxPath8] = {0};

    SHGetFolderPath(0, CSIDL_COMMON_APPDATA, 0, SHGFP_TYPE_CURRENT, path16);
    UTF8::Encode(path16, -1, path, kMaxPath8);
    return path;
}

std::string SysInfo::CommonAppDataPath(const char * name)
{
    auto root = CommonAppDataPath();
    const char * parts[] = { root.data(), name };
    return Path::JoinPath(parts, 2);
}

std::string SysInfo::WindowsPath()
{
    wchar_t path16[kMaxPath16] = {0};
    char path[kMaxPath8] = {0};
    GetWindowsDirectory(path16, kMaxPath16);
    UTF8::Encode(path16, -1, path, kMaxPath8);
    return path;
}

std::string SysInfo::WindowsPath(const char * name)
{
    auto root = WindowsPath();
    const char * parts[] = { root.data(), name };
    return Path::JoinPath(parts);
}

bool SysInfo::IsElevated()
{
    bool elevated = false;
    HANDLE process_token = NULL;
    if(OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &process_token)) 
    {
        TOKEN_ELEVATION elevation = {0};
        DWORD info_size = sizeof(elevation);
        if(::GetTokenInformation(process_token, TokenElevation, &elevation, 
                                 info_size, &info_size)) 
        {
            elevated = elevation.TokenIsElevated != FALSE;
        }
    }
    if(process_token)
        CloseHandle(process_token);
    return elevated;
}

bool SysInfo::IsUserAdmin()
{
    BOOL is_admin = false;
    PSID admin_group = 0; 
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &admin_group))
    {
        if(!CheckTokenMembership(0, admin_group, &is_admin)) 
            is_admin = FALSE;
        FreeSid(admin_group);
    }
    return is_admin != FALSE;
}

bool SysInfo::IsWinNT5()
{
    uint32_t major = 0;
    uint32_t minor = 0;
    if (QueryOSVersion(major, minor))
        return false;
    return major == 5;
}

bool SysInfo::IsWinXP()
{
    uint32_t major = 0;
    uint32_t minor = 0;
    if (QueryOSVersion(major, minor))
        return false;
    return major == 5 && minor == 1;
}

bool SysInfo::IsWin2K3()
{
    uint32_t major = 0;
    uint32_t minor = 0;
    if (QueryOSVersion(major, minor))
        return false;
    return major == 5 && minor == 2;
}

bool SysInfo::IsWinNT6()
{
    uint32_t major = 0;
    uint32_t minor = 0;
    if (QueryOSVersion(major, minor))
        return false;
    return major == 6;
}

bool SysInfo::IsX86()
{
    SYSTEM_INFO si = {0};
    GetNativeSystemInfo(&si);
    return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL;
}

bool SysInfo::IsX64()
{
    SYSTEM_INFO si = {0};
    GetNativeSystemInfo(&si);
    return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

std::string SysInfo::GetPlatformName()
{
    std::string os = "Unknown";
    std::string processor = "@Unknown";//32位为空 64位为@64 其他@Unknown;

    uint32_t major = 0;
    uint32_t minor = 0;
    QueryOSVersion(major, minor);

    SYSTEM_INFO si = {0};
    GetNativeSystemInfo(&si);
    if(PROCESSOR_ARCHITECTURE_INTEL == si.wProcessorArchitecture)
        processor = "";
    else if(PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture)
        processor = "@64";

    switch(major)
    {
    case 6:
        {
            switch(minor)
            {
            case 3://Windows 8.1 Windows Server 2012 R2
                 os = "Win8.1";
                break;
            case 2:
                os = "Win8";
                break;
            case 1:
                os = "Win7";
                break;
            case 0:
                os = "WinVista";
                break;
            }
        }
        break;
    case 5:
        {
            switch (minor)
            {
            case 2:
                os = "Win2K3";
                break;
            case 1:
                os = "WinXP";
                break;
            case 0:
                os = "Win2K";
                break;
            }
        }
        break;
    }
    
    return os + processor;
}

}