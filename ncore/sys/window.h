#ifndef NCORE_SYS_WINDOW_H_
#define NCORE_SYS_WINDOW_H_

#include "../base/object.h"
#include "../base/point.h"
#include "../base/size.h"
#include "../base/rect.h"
#include "../utils/disallow_copy_and_assign.h"

#include "window_define.h"

namespace ncore
{
namespace sys
{

class Window;

class WindowProperty
{
public:
    WindowProperty();

    const wchar_t * classname() const;
    uint32_t window_style() const;
    uint32_t window_style_ex() const;
    const Window * parent_window() const;

    void set_classname(const wchar_t * classname);
    void set_window_style(const uint32_t style);
    void set_window_style_ex(const uint32_t style);
    void set_parent_window(const Window * parent);

private:
    const wchar_t * classname_;
    uint32_t window_style_;
    uint32_t window_style_ex_;
    const Window * parent_window_;
};

struct WindowMessage
{
    uint32_t id;
    union
    {
        int32_t wcode;
        void * wval;
    };
    union
    {
        int32_t lcode;
        void * lval;
    };
    union
    {
        int32_t rcode;
        void * rval;
    };
};

class Window : public base::BaseObject
{
public:
    Window();

    Window(Window && object);

    virtual ~Window();

    bool IsValid();

    bool Create(const std::string & window_name,
                const base::Point & loc,
                const base::Size & size,
                const WindowProperty & window_property);

    bool Destroy();

    bool Attach(HWND handle);

    bool Show(bool value);
    bool Show(WindowShowMethod::Value cmd);

    bool Close();

    bool GetClientRect(base::Rect & rect);
    bool GetWindowRect(base::Rect & rect);

    bool Minimized();
    bool Maximized();

    bool SetLeft(int x);
    bool SetTop(int y);
    bool SetPos(int x, int y);
    bool SetPos(const base::Point & pos);

    bool SetWidth(int cx);
    bool SetHeight(int cy);
    bool SetSize(int cx, int cy);
    bool SetSize(const base::Size & size);

    bool InvalidateRect(bool erase);
    bool InvalidateRect(const base::Rect & rect, bool erase);

    void SetCapture();
    void ReleaseCapture();

    bool SetFocus();

    bool EnableWindow(bool enable);

    bool HasFocus() const;

    bool BringToForeground();

    bool IsVisible();

    bool IsEnabled();

    Window GetTopOwnedWindow();

    bool Post(const WindowMessage & wm);

    bool Send(WindowMessage & wm);

#ifdef NCORE_WINDOWS
    HWND NativeHandle() const;
#endif

protected:
#ifdef NCORE_WINDOWS
    HWND handle_;
#endif

private:
    DISALLOW_COPY_AND_ASSIGN(Window);
};

}

}

#endif