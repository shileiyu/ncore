#ifndef NCORE_BASE_EXCEPTION_H_
#define NCORE_BASE_EXCEPTION_H_

#include <ncore/ncore.h>

namespace ncore
{

class Exception
{
public:
    Exception();
    Exception(const char * message);
    virtual ~Exception();

    const char * message() const;
    void set_message(const char * message);
private:
    char message_[255];
    char tail_;
};

}

#endif