#ifndef NCORE_ENCODING_UTF8_H_
#define NCORE_ENCODING_UTF8_H_

#include <ncore/ncore.h>

namespace ncore
{

class UTF8
{
public:
    static int Decode(const char * multibyte,		/*source*/
                      const int multibyte_size,	    /*source size in character*/
                      wchar_t * widechar,			/*destnation*/
                      int widechar_size);			/*widechar size in character*/

    static int Encode(const wchar_t * widechar,	/*source*/
                      const int widechar_size,	/*source size in character*/
                      char * multibyte,			/*destnation*/
                      int multibyte_size);		/*multibyte size in character*/
};

}
#endif