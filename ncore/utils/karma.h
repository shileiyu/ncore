#ifndef NCORE_UTILS_KARMA_H_
#define NCORE_UTILS_KARMA_H_

#include <ncore/ncore.h>

/*
The name Karma just bring from boost library. It currently just a wrap of
standard library.
*/
namespace ncore
{

class Karma
{
public:
    template<typename T, size_t Size>
    static int Format(T (&dst)[Size], const T * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        return Format(dst, Size, fmt, args);
    }

    static int Format(char * dst, size_t size, const char * fmt, va_list args);

    static int Format(wchar_t * dst, size_t size, const wchar_t * fmt, va_list args);

    static std::string Format(const char * fmt, ...);

    static std::wstring Format(const wchar_t * fmt, ...);

    template<typename T, size_t Size>
    static T * Copy(T (&dst)[Size], const T * src)
    {
        return Copy(dst, Size, src);
    }

    static char * Copy(char * dst, size_t count, const char * src);

    static wchar_t * Copy(wchar_t * dst, size_t count, const wchar_t * src);

    static std::wstring FromUTF8(const std::string & utf8);

    static std::wstring FromUTF8(const char * utf8);

    static std::wstring FromUTF8(const char * utf8, size_t length);

    static std::string ToUTF8(const std::wstring & wide);

    static std::string ToUTF8(const wchar_t * wide);

    static std::string ToUTF8(const wchar_t * wide, size_t length);

    static std::string ToString(int64_t);

    static int64_t ToInt64(const std::string & str);

    static uint64_t ToUInt64(const std::string & str);

    static int Compare(const std::string& s1,
                       const std::string& s2,
                       bool ingore_case);
};

}

#endif