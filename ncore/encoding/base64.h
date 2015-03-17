#ifndef NCORE_ENCODING_BASE64_H_
#define NCORE_ENCODING_BASE64_H_

#include <ncore/ncore.h>

namespace ncore
{


class Base64
{
private:
    static const char kBase64[];
    static const char kPad64;
public:
    static int Encode(const void * blob,
                      size_t blob_size, 
                      char * code, 
                      size_t code_size);

    static int Decode(const char * code,
                      void * blob, 
                      size_t blob_size);
};


}

#endif