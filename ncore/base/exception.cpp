#include "exception.h"


namespace ncore
{

Exception::Exception()
    : tail_(0)
{
    memset(message_, 0, sizeof(message_));
}

Exception::Exception(const char * message)
    : tail_(0)
{
    set_message(message);
}

Exception::~Exception()
{
}

const char * Exception::message() const
{
    return message_;
}

void Exception::set_message(const char * message)
{
    if(message)
        strncpy_s(message_, message, sizeof(message_));
}

}