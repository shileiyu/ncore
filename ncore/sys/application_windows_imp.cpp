#include <ncore/encoding/utf8.h>
#include <ncore/utils/karma.h>
#include "application.h"


namespace ncore
{


std::string Application::GetModuleFullName()
{
    wchar_t filename16[kMaxPath16] = {0};
    char filename[kMaxPath8] = {0};
    std::string fullname;

    HMODULE module = reinterpret_cast<HMODULE>(Application::instance());
    ::GetModuleFileName(module, filename16, kMaxPath16);

    if(!UTF8::Encode(filename16, -1, filename, kMaxPath8))
        return fullname;

    return fullname.assign(filename);
}

std::string Application::GetExecutableFullName()
{
    wchar_t filename16[kMaxPath16] = {0};
    char filename[kMaxPath8] = {0};
    std::string fullname;

    ::GetModuleFileName(0, filename16, kMaxPath16);

    if(!UTF8::Encode(filename16, -1, filename, kMaxPath8))
        return fullname;

    return fullname.assign(filename);
}

void * Application::instance()
{
    HMODULE inst = 0;
    const LPCTSTR address = reinterpret_cast<LPCTSTR>(Application::instance);
    const DWORD query_flag = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

    ::GetModuleHandleEx(query_flag, address, &inst);
    return inst;
}

void Application::Exit(int exit_code)
{
    ::ExitProcess(exit_code);
}

void Application::Terminate(int exit_code)
{
    ::TerminateProcess(::GetCurrentProcess(), exit_code);
}

const Options & Application::GetOptions()
{
    std::string cmdline = Karma::ToUTF8(::GetCommandLine());
    static Options options = ParseOptions(cmdline);
    return options;
}


}