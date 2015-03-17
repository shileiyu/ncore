#ifndef NCORE_SYS_PROACTOR_H_
#define NCORE_SYS_PROACTOR_H_

#include <ncore/base/object.h>

namespace ncore
{

class IOPortal;

//前摄器
class Proactor : public NonCopyableObject
{
public:
    Proactor();
    ~Proactor();	
    bool init();
    void fini();

    //等待一个异步结果
    void Run(int ms);

    //关联到前摄器
    bool Associate(IOPortal & portal);

private:
#if defined NCORE_WINDOWS
    HANDLE comp_port_;
#endif
};


}

#endif