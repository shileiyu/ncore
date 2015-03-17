#include "utf8.h"

namespace ncore
{


#if defined NCORE_WINDOWS

int UTF8::Decode(const char * multibyte,    /*source*/
                 const int multibyte_size,  /*source size in character*/
                 wchar_t * widechar,        /*destnation*/
                 int widechar_size)         /*widechar size in character*/
{
    int written = MultiByteToWideChar(CP_UTF8, 0, multibyte, multibyte_size,
                                      widechar, widechar_size);
    if(written > 0 && written < widechar_size)
        widechar[written] = 0;
    return written;
}

int UTF8::Encode(const wchar_t * widechar,
                 const int widechar_size,
                 char * multibyte,
                 int multibyte_size)
{
    int written = WideCharToMultiByte(CP_UTF8, 0, widechar, widechar_size,
                                      multibyte, multibyte_size, NULL, NULL);
    if(written > 0 && written < multibyte_size)
        multibyte[written] = 0;
    return written;
}

#elif
  #error "Not suppport yet!"
#endif


}