#ifndef NCORE_SYS_DIALOG_BASE_H_
#define NCORE_SYS_DIALOG_BASE_H_

#include "window_base.h"

namespace ncore
{
namespace sys
{

class DialogBase : public WindowBase
{
public:
    DialogBase();
    virtual ~DialogBase();

    void OK();
    void Cancel();

    virtual bool WndProc(WindowMessage & wm) override;

    //模态对话框操作方法
    int DoModal();
    void EndDialog(int result);

private:
    DISALLOW_COPY_AND_ASSIGN(DialogBase);

private:
    int result_;
    bool stop_signal_;
public:
    utils::EventDelegate<EventArgs> OnOK;
    utils::EventDelegate<EventArgs> OnCancel;
};

}
}

#endif