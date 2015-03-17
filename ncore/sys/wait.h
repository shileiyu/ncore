#ifndef NCORE_SYS_WAIT_H_
#define NCORE_SYS_WAIT_H_

#include <ncore/ncore.h>
#include "waitable.h"

namespace ncore
{

class Wait
{
public:
    static const uint32_t kInfinity = -1;
    static bool WaitOne(Waitable & object);
    static bool WaitOne(Waitable & object, uint32_t time_out);
    static bool WaitOne(Waitable & object, uint32_t time_out, bool alertable);
};

}

#endif