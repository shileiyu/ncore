#ifndef NCORE_UTILS_WOW64_HELPER_H_
#define NCORE_UTILS_WOW64_HELPER_H_

#include <ncore/ncore.h>

namespace ncore
{

class DisableWow64FsRedirection
{
public:
    DisableWow64FsRedirection();

    ~DisableWow64FsRedirection();
private:
    void * old_value_;
};

}
#endif