#ifndef NCORE_SYS_APPLICATION_H_
#define NCORE_SYS_APPLICATION_H_

#include <ncore/ncore.h>
#include "options_parser.h"

namespace ncore
{


class Application
{
public:
    static std::string GetModuleFullName();

    static std::string GetExecutableFullName();

    static void * instance();

    static void Exit(int exit_code);

    static void Terminate(int exit_code);
    //获取进程命令行参数
    static const Options & GetOptions();
private:
    /*
    Application apply monostate pattern.
    don't need to create a instance.
    */
    Application();
    ~Application();
};


}

#endif