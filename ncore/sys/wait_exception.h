#ifndef NCORE_SYS_WAIT_EXCEPTION_H_
#define NCORE_SYS_WAIT_EXCEPTION_H_

#include <ncore/base/exception.h>

namespace ncore
{


class WaitExceptionFailed : public Exception
{
};

class WaitExceptionAbandon : public Exception
{
};


}

#endif 