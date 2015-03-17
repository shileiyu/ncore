#ifndef NCORE_SYS_ASYNC_CONTEXT_H_
#define NCORE_SYS_ASYNC_CONTEXT_H_

#include <ncore/ncore.h>

namespace ncore
{


class NamedEvent;

class AsyncContext
{
protected:
    AsyncContext();
    AsyncContext(const NamedEvent & e);
public:
    void * user_token() const;

    void set_user_token(void * token);

    void SuppressIOCP();
protected:
    OVERLAPPED overlapped_;
    void * user_token_;
};


}

#endif