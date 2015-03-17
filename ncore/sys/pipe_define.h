#ifndef NCORE_SYS_NAMED_PIPE_DEFINE_H_
#define NCORE_SYS_NAMED_PIPE_DEFINE_H_

#include <ncore/ncore.h>

namespace ncore
{

enum PipeDirection
{
    kIn = PIPE_ACCESS_INBOUND,
    kOut = PIPE_ACCESS_OUTBOUND,
    kDuplex =PIPE_ACCESS_DUPLEX,
};

enum PipeOption
{
    kNone = 0,
    kWriteThrough = FILE_FLAG_WRITE_THROUGH,
};

enum PipeTransmissionMode
{
    kStream = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
    kMessage = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
};

}

#endif