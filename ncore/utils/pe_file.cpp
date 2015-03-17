#include <ncore/encoding/utf8.h>
#include "pe_file.h"

namespace ncore
{

bool PEFile::GetFileVersion(const char * file, uint64_t & ver)
{
    static const size_t kMaxPath = 260;

    wchar_t filename16[kMaxPath];

    if(!UTF8::Decode(file, -1, filename16, kMaxPath))
        return false;

    DWORD handle = 0;
    DWORD size = ::GetFileVersionInfoSize(filename16, &handle);
    if (size == 0)
        return false;

    auto buf = std::make_unique<uint8_t[]>(size);

    if(!::GetFileVersionInfo(filename16, 0, size, buf.get()))
        return false;

    VS_FIXEDFILEINFO * info_ptr = 0;
    uint32_t info_len = 0;
    if(!::VerQueryValue(buf.get(), L"\\", &(PVOID&)info_ptr, &info_len))
        return false;

    ver = info_ptr->dwFileVersionMS;
    ver <<= 32;
    ver |= info_ptr->dwFileVersionLS;
    return true;
}

}