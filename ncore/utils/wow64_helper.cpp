#include "wow64_helper.h"

namespace ncore
{


DisableWow64FsRedirection::DisableWow64FsRedirection()
    :old_value_(0)
{
    typedef BOOL (WINAPI *Wow64DisableWow64FsRedirection_T)(PVOID *OldValue);
    static Wow64DisableWow64FsRedirection_T TDisableWow64FsRedirection = 
        (Wow64DisableWow64FsRedirection_T)::GetProcAddress(
        ::GetModuleHandle(L"kernel32"), 
        "Wow64DisableWow64FsRedirection");
    if(TDisableWow64FsRedirection)
        TDisableWow64FsRedirection(&old_value_);
}

DisableWow64FsRedirection::~DisableWow64FsRedirection()
{
    typedef BOOL (WINAPI *Wow64RevertWow64FsRedirection_T)(PVOID OldValue);
    static Wow64RevertWow64FsRedirection_T TWow64RevertWow64FsRedirection = 
        (Wow64RevertWow64FsRedirection_T)::GetProcAddress(
        ::GetModuleHandle(L"kernel32"), 
        "Wow64RevertWow64FsRedirection");

    if(TWow64RevertWow64FsRedirection)
        TWow64RevertWow64FsRedirection(old_value_);
}


}