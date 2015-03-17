#ifndef NCORE_SYS_WINDOW_DEFINE_H_
#define NCORE_SYS_WINDOW_DEFINE_H_

namespace ncore
{
namespace sys
{

namespace WindowPlacement
{
enum Value
{
    kOrdinary,
    kMinimized,
    kMaxmized,
};
}

namespace WindowShowMethod
{
enum Value
{
    kShow               = SW_SHOW,
    kHide               = SW_HIDE,
    kShowNormal         = SW_SHOWNORMAL,
    kShowRestore        = SW_RESTORE,
    kShowMinimized      = SW_SHOWMINIMIZED,
    kShowMaximized      = SW_SHOWMAXIMIZED,
    kShowNoActive       = SW_SHOWNOACTIVATE,
};
}

}
}

#endif