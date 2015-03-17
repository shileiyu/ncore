#include "../utils/karma.h"
#include "application.h"
#include "window.h"

namespace ncore
{
namespace sys
{

/*/////////////////////////////////////////////////////////////////////////////
//窗体属性Windows实现，窗体属性包括如下属性
//1.窗体类属性
//2.窗体风格
//3.窗体风格扩展
//4.父窗口或OwnerWindow
/////////////////////////////////////////////////////////////////////////////*/
WindowProperty::WindowProperty()
{
    classname_ = 0;
    window_style_ = WS_OVERLAPPEDWINDOW;
    window_style_ex_ = 0;
    parent_window_ = 0;
}

const wchar_t * WindowProperty::classname() const 
{
    return classname_;
}

uint32_t WindowProperty::window_style() const 
{
    return window_style_;
}

uint32_t WindowProperty::window_style_ex() const 
{
    return window_style_ex_;
}

const Window * WindowProperty::parent_window() const
{
    return parent_window_;
}

void WindowProperty::set_classname(const wchar_t * classname)
{
    classname_ = classname;
}

void WindowProperty::set_window_style(const uint32_t style) 
{ 
    window_style_ = style;
}

void WindowProperty::set_window_style_ex(const uint32_t style) 
{ 
    window_style_ex_ = style;
}

void WindowProperty::set_parent_window(const Window * parent) 
{ 
    parent_window_ = parent;
}
///////////////////////////////////////////////////////////////////////////////

Window::Window()
{
    handle_ = 0;
}

Window::Window(Window && object)
{
    std::swap(handle_, object.handle_);
}

Window::~Window()
{
}

bool Window::IsValid()
{
    return handle_ != NULL;
}

bool Window::Create(const std::string & window_name,
                    const base::Point & loc,
                    const base::Size & size,
                    const WindowProperty & window_property)
{
    using namespace utils;

    HINSTANCE inst = reinterpret_cast<HINSTANCE>(Application::instance());


    auto prop = static_cast<const WindowProperty &>(window_property);

    const wchar_t * classname = prop.classname();
    const Window * parent_window = prop.parent_window();
    HWND parent_window_handle = 0;
    uint32_t wsex = prop.window_style_ex();
    uint32_t ws = prop.window_style();
    int x = loc.x();
    int y = loc.y();
    int cx = size.width();
    int cy = size.height();

    if(parent_window)
        parent_window_handle = parent_window->handle_;

    auto window_name16 = Karma::FromUTF8(window_name.data());
    handle_ = CreateWindowEx(wsex, classname, window_name16.data(), ws, x, y, 
                             cx, cy, parent_window_handle, 0, inst, this);

    if(handle_ == 0)
        return false;
    
    return true;
}

bool Window::Show(bool value)
{
    using namespace WindowShowMethod;

    //kShow wouldn't change window placement.
    return value ? Show(kShow) : Show(kHide);
}

bool Window::Show(const WindowShowMethod::Value cmd)
{
    using namespace WindowShowMethod;

    if(!handle_)
        return false;

    return ::ShowWindow(handle_, cmd) != FALSE;
}

bool Window::Destroy()
{
    bool result = false;
    if( handle_ != 0 && 
        handle_ != HWND_BROADCAST && 
        handle_ != HWND_MESSAGE)
    {
        result = ::DestroyWindow(handle_) != FALSE;
        handle_ = 0;
    }
    return result;;
}

bool Window::Close()
{
    bool result = false;
    if(handle_)
    {
        result = ::CloseWindow(handle_) != FALSE;
    }
    return result;;
}

bool Window::GetClientRect(base::Rect & rect)
{
    RECT client_rect;
    bool result = false;

    if(handle_)
    {
        result = ::GetClientRect(handle_, &client_rect) != FALSE;
        rect.SetLTRB(client_rect.left, client_rect.top,
            client_rect.right, client_rect.bottom);
    }
    return result;
}

bool Window::GetWindowRect(base::Rect & rect)
{
    RECT client_rect;
    bool result = false;

    if(handle_)
    {
        result = ::GetWindowRect(handle_, &client_rect) != FALSE;
        rect.SetLTRB(client_rect.left, client_rect.top,
            client_rect.right, client_rect.bottom);
    }
    return result;
}

bool Window::Minimized()
{
    bool result = false;

    if(handle_)
    {
        result = ::IsMinimized(handle_) != FALSE;
    }
    return result;
}

bool Window::Maximized()
{
    bool result = false;

    if(handle_)
    {
        result = ::IsMaximized(handle_) != FALSE;
    }
    return result;
}

bool Window::SetLeft(int x)
{
    base::Rect wr;
    if(!GetWindowRect(wr))
        return false;
    return SetPos(x, wr.top());
}

bool Window::SetTop(int y)
{
    base::Rect wr;
    if(!GetWindowRect(wr))
        return false;
    return SetPos(wr.left(), y);
}

bool Window::SetPos(const base::Point & pos)
{
    return SetPos(pos.x(), pos.y());
}

bool Window::SetPos(int x, int y)
{
    uint32_t flag = SWP_NOSIZE | SWP_NOZORDER | 
        SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS;
    if(!handle_)
        return false;
    return ::SetWindowPos(handle_, 0, x, y, 0, 0, flag) != FALSE;
}

bool Window::SetWidth(int cx)
{
    base::Rect wr;
    if(!GetWindowRect(wr))
        return false;
    return SetSize(cx, wr.height());
}

bool Window::SetHeight(int cy)
{
    base::Rect wr;
    if(!GetWindowRect(wr))
        return false;
    return SetSize(wr.width(), cy);
}

bool Window::SetSize(const base::Size & size)
{
    return SetSize(size.width(), size.height());
}

bool Window::SetSize(int cx, int cy)
{
    uint32_t flag = SWP_NOMOVE | SWP_NOZORDER | 
        SWP_NOACTIVATE |SWP_ASYNCWINDOWPOS;

    if(!handle_)
        return false;
    return ::SetWindowPos(handle_, 0, 0, 0, cx, cy, flag) != FALSE;
}

bool Window::InvalidateRect(bool erase)
{
    if(!handle_)
        return false;
    return ::InvalidateRect(handle_, 0, erase) != FALSE;
}

bool Window::InvalidateRect(const base::Rect & rect, bool erase)
{
    if(!handle_)
        return false;

    RECT r = {rect.left(), rect.top(), rect.right(), rect.bottom()};
    return ::InvalidateRect(handle_, &r, erase) != FALSE;
}

void Window::SetCapture()
{
    if(!handle_)
        return;
    ::SetCapture(handle_);
}

void Window::ReleaseCapture()
{
    if(!handle_)
        return;
    ::ReleaseCapture();
}

bool Window::SetFocus()
{
    if(handle_)
    {
        return ::SetFocus(handle_) != 0;
    }
    return false;
}

bool Window::EnableWindow(bool enable)
{
    if(handle_)
    {
        return ::EnableWindow(handle_, enable) != FALSE;
    }
    return false;
}

bool Window::HasFocus() const
{
    if(handle_)
    {
        if(::GetFocus() == handle_)
            return true;
    }
    return false;
}

bool Window::BringToForeground()
{
    if(handle_)
    {
        return ::SetForegroundWindow(handle_) != FALSE;
    }
    return false;
}

bool Window::IsVisible()
{
    if(handle_)
    {
        return IsWindowVisible(handle_) != FALSE;
    }
    return false;
}

bool Window::IsEnabled()
{
    if(handle_)
    {
        return IsWindowEnabled(handle_) != FALSE;
    }
    return false;
}

Window Window::GetTopOwnedWindow()
{
    Window top;
    if(handle_)
    {
        HWND popup = GetWindow(handle_, GW_ENABLEDPOPUP);
        HWND first = GetWindow(popup, GW_HWNDFIRST);
        top.Attach(first);
    }
    return std::move(top);
}

HWND Window::NativeHandle() const
{
    return handle_;
}

bool Window::Attach(HWND handle)
{
    if( ::IsWindow(handle) || 
        handle == HWND_BROADCAST ||
        handle == HWND_MESSAGE )
    {
        handle_ = handle;
        return true;
    }
    return false;
}

bool Window::Post(const WindowMessage & wm)
{
    if(!handle_)
        return false;

    return ::PostMessage(handle_, wm.id, wm.wcode, wm.lcode) != FALSE;
}

bool Window::Send(WindowMessage & wm)
{
    if(!handle_)
        return false;

    wm.rcode = ::SendMessage(handle_, wm.id, wm.wcode, wm.lcode);

    auto err = ::GetLastError();

    switch(err)
    {
    case ERROR_ACCESS_DENIED:
    case ERROR_INVALID_WINDOW_HANDLE:
        return false;
    default:
        return true;
    }
}

}
}