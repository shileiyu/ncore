#include "application.h"
#include "message_loop.h"
#include "sys_info.h"
#include "dialog_base.h"

namespace ncore
{
namespace sys
{

DialogBase::DialogBase()
    : stop_signal_(false), result_(0)
{

}

DialogBase::~DialogBase()
{
}

int DialogBase::DoModal()
{
    if(handle_)
    {
        MessageLoop * cur_loop = MessageLoop::Current();
        //检测Owner窗口
        HWND owner = ::GetWindow(handle_, GW_OWNER);
        //如果是桌面则不禁用
        if(owner == GetDesktopWindow())
            owner = 0;

        stop_signal_ = false;
        //使自己可见
        this->Show(true);
        //禁用Owner
        if(owner)
            ::EnableWindow(owner, false);

        //进入模态循环
        if(cur_loop)
        {
            cur_loop->Run(true, stop_signal_);
        }
        else
        {
            MessageLoop local_loop;
            local_loop.Run(true, stop_signal_);
        }

        //重新启用Owner
        if(owner)
        {
            ::EnableWindow(owner, true);
            ::SetActiveWindow(owner);
        }
        //关闭自己
        this->Show(false);
    }
    return result_;
}

void DialogBase::EndDialog(int result)
{
    result_ = result;
    stop_signal_ = true;
}

void DialogBase::OK()
{
    if(OnOK != nullptr)
    {
        OnOK(this, EventArgs());
    }
    else
    {
        EndDialog(0);
    }
}

void DialogBase::Cancel()
{
    if(OnCancel != nullptr)
    {
        OnCancel(this, EventArgs());
    }
    else
    {
        EndDialog(-1);
    }
}

bool DialogBase::WndProc(WindowMessage & wm)
{
    if(wm.id == WM_SYSCOMMAND && wm.wcode == SC_CLOSE)
    {
        wm.rcode = 0;
        return true;
    }

    return WindowBase::WndProc(wm);
}

}
}