#ifndef NCORE_PE_FILE_H_
#define NCORE_PE_FILE_H_

#include <ncore/ncore.h>

namespace ncore
{


class PEFile
{
public:
    static bool GetFileVersion(const char * file, uint64_t & ver);
};

}

#endif
