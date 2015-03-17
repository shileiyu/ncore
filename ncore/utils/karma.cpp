#include <ncore/encoding/utf8.h>
#include "handy.h"
#include "karma.h"

namespace ncore
{


int Karma::Format(char * dst, size_t size, const char * fmt, va_list args)
{
    return vsprintf_s(dst, size, fmt, args);
}

int Karma::Format(wchar_t * dst, size_t size, const wchar_t * fmt, va_list args)
{
    return vswprintf_s(dst, size, fmt, args);
}

std::string Karma::Format(const char * fmt, ...)
{
    static const size_t kStackSize = 0x400;
    char stack_buf[kStackSize];
    char * buf = stack_buf;
    size_t cap = kStackSize;
    std::string formated;
    if(!fmt)
        return formated;
    
    va_list args;
    va_start(args, fmt);
    int length = _vscprintf(fmt, args);
    va_end(args);

    if(length <= 0)
        return formated;

    if(length >= kStackSize)
    {
        cap = length + 4;
        buf = new char[cap];
    }

    if(!buf)
        return formated;

    va_start(args, fmt);
    vsprintf_s(buf, cap, fmt, args);
    va_end(args); 

    formated.assign(buf, length);
    if(buf != stack_buf)
        delete buf;
    return formated;
}

std::wstring Karma::Format(const wchar_t * fmt, ...)
{
    static const size_t kStackSize = 0x400;
    wchar_t stack_buf[kStackSize];
    wchar_t * buf = stack_buf;
    size_t cap = kStackSize;
    std::wstring formated;
    if(!fmt)
        return formated;
    
    va_list args;
    va_start(args, fmt);
    int length = _vscwprintf(fmt, args);
    va_end(args);

    if(length <= 0)
        return formated;

    if(length >= kStackSize)
    {
        cap = length + 4;
        buf = new wchar_t[cap];
    }
    if(!buf)
        return formated;

    va_start(args, fmt);
    vswprintf_s(buf, cap, fmt, args);
    va_end(args); 

    formated.assign(buf, length);
    if(buf != stack_buf)
        delete buf;
    return formated;
}

char * Karma::Copy(char * dst, size_t count, const char * src)
{
    strcpy_s(dst, count, src);
    return dst;
}

wchar_t * Karma::Copy(wchar_t * dst, size_t count, const wchar_t * src)
{
    wcscpy_s(dst, count, src);
    return dst;
}

std::wstring Karma::FromUTF8(const std::string & utf8)
{
    return FromUTF8(utf8.data(), utf8.length());
}

std::wstring Karma::FromUTF8(const char * utf8)
{
    return FromUTF8(utf8, strlen(utf8));
}

std::wstring Karma::FromUTF8(const char * utf8, size_t length)
{
    std::wstring wide;
    int wide_size = UTF8::Decode(utf8, length, 0, 0);
    if(!wide_size)
        return wide;

    auto scoped_buffer = std::make_unique<wchar_t[]>(wide_size);
    auto wide_buffer = scoped_buffer.get();
    wide_size = UTF8::Decode(utf8, length, wide_buffer, wide_size);
    length == -1 ? --wide_size : 0;
    wide.assign(wide_buffer, wide_size);
    return std::move(wide);
}

std::string Karma::ToUTF8(const std::wstring & wide)
{
    return ToUTF8(wide.data(), wide.length());
}

std::string Karma::ToUTF8(const wchar_t * wide)
{
    return ToUTF8(wide, wcslen(wide));
}

std::string Karma::ToUTF8(const wchar_t * wide, size_t length)
{
    std::string utf8;
    int utf8_size = UTF8::Encode(wide, length, 0, 0);
    if(!utf8_size)
        return utf8;
    
    auto scoped_buffer = std::make_unique<char[]>(utf8_size);
    auto utf8_buffer = scoped_buffer.get();
    utf8_size = UTF8::Encode(wide, length, utf8_buffer, utf8_size);
    length == -1 ? --utf8_size : 0;
    utf8.assign(utf8_buffer, utf8_size);
    return std::move(utf8);
}

std::string Karma::ToString(int64_t value)
{
    return Karma::Format("%I64d", value);
}

int64_t Karma::ToInt64(const std::string & str)
{
    char * end = nullptr;
    return _strtoi64(str.data(), &end, 10);
}

uint64_t Karma::ToUInt64(const std::string & str)
{
    char * end = nullptr;
    return _strtoui64(str.data(), &end, 10);
}

int Karma::Compare(const std::string& s1,
                   const std::string& s2,
                   bool ingore_case)
{
    auto str1 = s1.data();
    auto str2 = s2.data();

    if(ingore_case)
        return _stricmp(str1, str2);
    else
        return strcmp(str1, str2);
}

}