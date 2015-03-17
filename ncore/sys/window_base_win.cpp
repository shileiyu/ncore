#include "application.h"
#include "sys_info.h"
#include "spin_lock.h"
#include "window_base.h"

namespace ncore
{
namespace sys
{

static sys::SpinLock g_spin_lock;
static int g_wndclass_ref;
/*///////////////////////////////////////////////////////////////////////////////
//窗体Window实现
///////////////////////////////////////////////////////////////////////////////*/

const wchar_t * WindowBase::kClassName = L"NS_WINDOW";

bool WindowBase::RegWndClass()
{
    HINSTANCE inst = reinterpret_cast<HINSTANCE>(Application::instance());

    WNDCLASSEX wndcls = {0};
    wndcls.cbSize = sizeof(wndcls);
    bool succed = false;

    g_spin_lock.Acquire();
    if(GetClassInfoEx(inst, kClassName, &wndcls))
    {
        ++g_wndclass_ref;
        succed = true;
    }
    else
    {
        wndcls.cbSize = sizeof(wndcls);
        wndcls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wndcls.lpfnWndProc = StaticWindowProc;
        wndcls.cbClsExtra = 0;
        wndcls.cbWndExtra = 0;
        wndcls.hInstance = inst;
        wndcls.hIcon = 0;
        wndcls.hCursor = 0;
        wndcls.hbrBackground = 0;
        wndcls.lpszMenuName = 0;
        wndcls.lpszClassName = kClassName;
        wndcls.hIconSm = 0;
        if(::RegisterClassEx(&wndcls))
        {
            ++g_wndclass_ref;
            succed = true;
        }
    }
    g_spin_lock.Release();
    return succed;
}

bool WindowBase::UnregWndClass()
{
    HINSTANCE inst = reinterpret_cast<HINSTANCE>(Application::instance());
    g_spin_lock.Acquire();
    assert(g_wndclass_ref);
    if(--g_wndclass_ref == 0)
        ::UnregisterClass(kClassName, inst);
    g_spin_lock.Release();
    return true;
}

LRESULT _stdcall WindowBase::StaticWindowProc(HWND hWnd, UINT uMsg, 
                                              WPARAM wParam, LPARAM lParam)
{
    WindowMessage wm;

    wm.rcode = 0;
    if(uMsg == WM_NCCREATE && GetWindowLongPtr(hWnd, GWLP_USERDATA) == 0)
    {
        auto lcp = reinterpret_cast<LPCREATESTRUCT>(lParam);
        auto this_window = reinterpret_cast<WindowBase *>(lcp->lpCreateParams);

        this_window->handle_ = hWnd;

#if defined NCORE_X86
        LONG window_pointer = reinterpret_cast<LONG>(this_window);
#elif defined NCORE_X64
        LONG_PTR window_pointer = reinterpret_cast<LONG_PTR>(this_window);
#endif
        SetWindowLongPtr(hWnd, GWLP_USERDATA, window_pointer);
    }

    auto window_ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    auto this_window = reinterpret_cast<WindowBase *>(window_ptr);

    if(this_window)
    {
        wm.id = uMsg;
        wm.wcode = wParam;
        wm.lcode = lParam;

        this_window->WndProc(wm);
        
    }
    else
    {
        wm.rcode = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return wm.rcode;
}

WindowBase::WindowBase()
    :cursor_(kNoCursor),
     tracking_mouse_(false),
     placement_(WindowPlacement::kOrdinary)
{
}

WindowBase::~WindowBase()
{
    Destroy();
}

const base::Point & WindowBase::loc() const
{
    return loc_;
}

const base::Size & WindowBase::size() const
{
    return size_;
}

void WindowBase::SetCursor(CursorStyle style)
{
    static HCURSOR arrow = LoadCursor(0, IDC_ARROW);
    static HCURSOR hand = LoadCursor(0, IDC_HAND);
    static HCURSOR ibeam = LoadCursor(0, IDC_IBEAM);
    static HCURSOR wait = LoadCursor(0, IDC_WAIT);
    static HCURSOR forbid = LoadCursor(0, IDC_NO);
    static HCURSOR sizenwse = LoadCursor(0,IDC_SIZENWSE);

    switch(style)
    {
    case kNoCursor:
        ::SetCursor(0);
        break;
    case kArrow:
        ::SetCursor(arrow);
        break;
    case kHand:
        ::SetCursor(hand);
        break;
    case kIBeam:
        ::SetCursor(ibeam);
        break; 
    case kWait:
        ::SetCursor(wait);
        break;
    case kForbid:
        ::SetCursor(forbid);
        break;
    case kSizeNWSE:
        ::SetCursor(sizenwse);
        break;
    default:
        assert(0);
    }
}

void WindowBase::ChangeCursor(CursorStyle style)
{
    cursor_ = style;
}

void WindowBase::GetLocalMonitorRect(base::Rect & monitor_rect, 
                                     base::Rect & monitor_work_rect)
{
    if(handle_==0)
        return;

    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(MONITORINFO);

    HMONITOR hmonitor = MonitorFromWindow(handle_, MONITOR_DEFAULTTONEAREST);
    GetMonitorInfo(hmonitor, &monitor_info);

    monitor_rect.SetLTRB(
        monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
        monitor_info.rcMonitor.right, monitor_info.rcMonitor.bottom);
    monitor_work_rect.SetLTRB(
        monitor_info.rcWork.left, monitor_info.rcWork.top,
        monitor_info.rcWork.right, monitor_info.rcWork.bottom);
}

bool WindowBase::SetTimer(uint32_t timer_id,  uint32_t ms)
{
    assert(handle_);

    return ::SetTimer(handle_, timer_id, ms, 0) != 0;
}

bool WindowBase::KillTimer(uint32_t timer_id)
{
    assert(handle_);

    return ::KillTimer(handle_, timer_id) != FALSE;
}

bool WindowBase::ExtendFrameIntoClientArea(int left_width,
                                           int right_width,
                                           int top_height,
                                           int bottom_height)
{
    if(!SysInfo::DWMEnabled())
        return false;

    MARGINS margins = {left_width, right_width, top_height, bottom_height};
    HRESULT hr = S_OK;
    
    hr = DwmExtendFrameIntoClientArea(handle_, &margins);

    if (SUCCEEDED(hr))
        return true;
    else
        return false;
}

void WindowBase::DeterminePlacement()
{
    using namespace WindowPlacement;

    EventArgs e;
    WINDOWPLACEMENT wp = {0};
    wp.length = sizeof(wp);

    if(::GetWindowPlacement(handle_, &wp))
    {
        switch(wp.showCmd)
        {
        case SW_SHOWNORMAL:
            {
                if(placement_ != kOrdinary)
                    OnRestore(this, e);

                placement_ = kOrdinary;
            }
            break;
        case SW_SHOWMAXIMIZED:
            {
                if(placement_ != kMaxmized)
                    OnMaximized(this, e);

                placement_ = kMaxmized;
            }
            break;
        case SW_SHOWMINIMIZED:
            {
                if(placement_ != kMinimized)
                    OnMinimized(this, e);

                placement_ = kMinimized;
            }
            break;
        }
    }
    return;
}

bool WindowBase::WndProc(WindowMessage & wm)
{
    bool was_handled = false;
    auto inst = reinterpret_cast<HINSTANCE>(Application::instance());

    if( wm.id >= WM_MOUSEFIRST && 
        wm.id <= WM_MOUSELAST  ||
        wm.id == WM_MOUSELEAVE )
    {
        was_handled = HandleMouse(wm);
    }
    else if(wm.id >= WM_KEYFIRST && wm.id <= WM_KEYLAST)
    {
        was_handled = HandleKeyBoard(wm);
    }
    else
    {
        switch(wm.id)
        {
        case WM_CREATE:
            {
                was_handled = HandleCreate(wm);
            }
            break;
        case WM_DESTROY:
            {
                EventArgs e;
                OnDestroy(this, e);
                handle_ = 0;
            }
            break;
        case WM_CLOSE:
            {
                CancelArgs e;
                e.canceled = false;
                OnClose(this, e);
                if(e.canceled)
                {
                    wm.rcode = 0;
                    was_handled = true;
                }
            }
            break;
        case WM_PAINT:
            {
                was_handled = HandlePaint(wm);
            }
            break;
        case WM_SETCURSOR:
            {
                auto ht = LOWORD(wm.lcode);
                if(ht == HTCLIENT)
                {
                    wm.rcode = TRUE;
                    was_handled = true;
                }
            }
            break;
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
            {
                was_handled = HandlePositionChanges(wm);
            }
            break;
        case WM_TIMER:
            {
                was_handled = HandleTimer(wm);
            }
            break;
        case WM_GETMINMAXINFO:
            {
                was_handled = HandleMinMaxInfo(wm);
            }
            break;
        case WM_NCCALCSIZE:
            {
                was_handled = HandleNonClientSize(wm);
            }
            break;
        case WM_ACTIVATE:
            {
                ActivateParam ap = kInactive;

                switch(wm.wcode & 0xFFFF)
                {
                case WA_INACTIVE:
                    ap = kInactive;
                    break;
                case WA_ACTIVE:
                    ap = kActive;
                    break;
                case WA_CLICKACTIVE:
                    ap = kAcitveByClick;
                    break;
                default:
                    assert(0);
                }
                OnActivated(this, ap); 
            }
            break;
        case WM_SETFOCUS:
            {
                OnSetFocus(this, EventArgs());
            }
            break;
        case WM_KILLFOCUS:
            {
                OnKillFocus(this, EventArgs());
            }
            break;
        case WM_ENABLE:
            {
                EventArgs e;
                if(wm.wcode)
                    OnEnabled(this, e);
                else
                    OnDisable(this, e);

                was_handled = true;
                wm.rcode = 0;
            }
            break;
        default:
            {
                MessageArgs e = { wm, false }; 
                OnMessage(this, e);
                was_handled = e.handled;
            }
            break;
        }
    }

    if(!was_handled)
        DefWndProc(wm);

    return true;
}

bool WindowBase::DefWndProc(WindowMessage & wm)
{
    wm.rcode = ::DefWindowProc(handle_, wm.id, wm.wcode, wm.lcode);
    return true;
}

bool WindowBase::HandleCreate(WindowMessage & wm)
{
    assert(handle_);

    CREATESTRUCT * param = reinterpret_cast<CREATESTRUCT *>(wm.lcode);
    loc_.SetLoc(param->x, param->y);
    size_.SetSize(param->cx, param->cy);
    
    CancelArgs ce = { false };
    OnCreate(this, ce);
    if(!ce.canceled)
    {
        EventArgs e;
        OnPositionChanged(this, e);
        OnSizeChanged(this, e);
        cursor_ = kArrow;
        wm.rcode = 0;
    }
    else
    {
        wm.rcode = -1;
    }

    return true;
}

bool WindowBase::HandlePaint(WindowMessage & wm)
{
    assert(handle_);

    switch(wm.id)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps = {0};
            base::Rect rect;
            BeginPaint(handle_, &ps);
            rect. SetLTRB(ps.rcPaint.left, ps.rcPaint.top, 
                          ps.rcPaint.right, ps.rcPaint.bottom);

            if(OnPaint != nullptr)
            {
                PaintArgs e;
                e.dc = ps.hdc;
                e.rect = rect;
                OnPaint(this, e);
            }
            else
            {
                RECT fill;
                if(rect.isEmpty())
                {
                    ::GetClientRect(handle_, &fill);
                }
                else
                {
                    fill.left = rect.left();
                    fill.top = rect.top();
                    fill.right = rect.right();
                    fill.bottom = rect.bottom();
                }
                ::FillRect(ps.hdc, &fill, (HBRUSH)GetStockObject(0));
            }

            EndPaint(handle_, &ps);
        }
        break;
    default:
        assert(0);
    }
    wm.rcode = 0;
    return true;
}

bool WindowBase::HandlePositionChanges(WindowMessage & wm)
{
    assert(handle_);

    WINDOWPOS * new_wnd_pos = reinterpret_cast<WINDOWPOS *>(wm.lcode);
    base::Point new_pos(new_wnd_pos->x, new_wnd_pos->y);
    base::Size  new_size(new_wnd_pos->cx, new_wnd_pos->cy);

    switch(wm.id)
    {
    case WM_WINDOWPOSCHANGING:
        {
            PositionChangingArgs pe = { new_pos, false };
            OnPositionChanging(this, pe);
            if(pe.denied)
            {
                new_wnd_pos->flags |= SWP_NOREPOSITION;
            }
            else
            {
                new_wnd_pos->x = new_pos.x();
                new_wnd_pos->y = new_pos.y();
            }
            SizeChangingArgs se = { new_size, false };
            OnSizeChanging(this, se);
            if(se.denied)
            {
                new_wnd_pos->flags |= SWP_NOSIZE;
            }
            else
            {
                new_wnd_pos->cx = new_size.width();
                new_wnd_pos->cy = new_size.height();
            }
        }
        break;
    case WM_WINDOWPOSCHANGED:
        {
            EventArgs e;
            if(!(new_wnd_pos->flags & SWP_NOMOVE))
            {
                if(loc_ != new_pos)
                {
                    loc_ = new_pos;
                    OnPositionChanged(this, e);
                }
            }
            
            if(!(new_wnd_pos->flags & SWP_NOSIZE))
            {
                if(size_ != new_size)
                {
                    size_ = new_size;
                    OnSizeChanged(this, e);
                }
            }
        }
        break;
    }

    DeterminePlacement();
    wm.rcode = 0;
    return true;
}

bool WindowBase::HandleMouse(WindowMessage & wm)
{
    assert(handle_);

    int16_t x = static_cast<int16_t>(wm.lcode);
    int16_t y = static_cast<int16_t>(wm.lcode >> 16);
    MouseEvent e(x, y);

    MouseEvent::MouseButtons button = 0;

    if(wm.wcode & MK_LBUTTON)
        button |= MouseEvent::kLeft;
    if(wm.wcode & MK_RBUTTON)
        button |= MouseEvent::kRight;
    if(wm.wcode & MK_MBUTTON)
        button |= MouseEvent::kMiddle;
    if(wm.wcode & MK_XBUTTON1)
        button |= MouseEvent::kXButton1;
    if(wm.wcode & MK_XBUTTON2)
        button |= MouseEvent::kXButton2;

    e.set_state(button);

    if( wm.id != WM_MOUSELEAVE &&
        tracking_mouse_ == false)
    {
        //开启鼠标追踪
        TRACKMOUSEEVENT track_event = 
        {
            sizeof(track_event),
            TME_LEAVE,
            handle_,
            HOVER_DEFAULT
        };

        tracking_mouse_ = TrackMouseEvent(&track_event) != FALSE;
        e.set_type(MouseEvent::kMouseEnter);
        OnMouseEnter(this, e);
    }

    switch(wm.id)
    {
    case WM_MOUSEMOVE:
        {
            e.set_type(MouseEvent::kMouseMove);
            OnMouseMove(this, e);
            SetCursor(cursor_);
        }
        break;
    case WM_LBUTTONDBLCLK:
        {
            e.set_button(MouseEvent::kLeft);
            e.set_type(MouseEvent::kMouseDoubleClick);
            OnMouseDoubleClick(this, e);
        }
        break;
    case WM_LBUTTONDOWN:
        {
            e.set_button(MouseEvent::kLeft);
            e.set_type(MouseEvent::kMouseDown);
            OnMouseDown(this, e);
        }
        break;
    case WM_LBUTTONUP:
        {   
            e.set_button(MouseEvent::kLeft);
            e.set_type(MouseEvent::kMouseUp);
            OnMouseUp(this, e);
        }
        break;
    case WM_RBUTTONDBLCLK:
        {
            e.set_button(MouseEvent::kRight);
            e.set_type(MouseEvent::kMouseDoubleClick);
            OnMouseDoubleClick(this, e);
        }
        break;
    case WM_RBUTTONDOWN:
        {
            e.set_button(MouseEvent::kRight);
            e.set_type(MouseEvent::kMouseDown);
            OnMouseDown(this, e);
        }
        break;
    case WM_RBUTTONUP:
        {
            e.set_button(MouseEvent::kRight);
            e.set_type(MouseEvent::kMouseUp);
            OnMouseUp(this, e);
        }
        break;
    case WM_MOUSELEAVE:
        {
            e.set_type(MouseEvent::kMouseLeave);
            OnMouseLeave(this, e);
            tracking_mouse_ = false;
        }
        break;
    case WM_MOUSEWHEEL:
        {
            e.set_dy(HIWORD(wm.wcode));
            e.set_type(MouseEvent::kMouseWheel);
            OnMouseWheel(this, e);
        }
        break;
    case WM_MOUSEHWHEEL:
        {
            e.set_dx(HIWORD(wm.wcode));
            e.set_type(MouseEvent::kMouseWheel);
            OnMouseWheel(this, e);
        }
        break;
    }
    wm.rcode = 0;
    return true;
}

bool WindowBase::HandleKeyBoard(WindowMessage & wm)
{
    assert(handle_);

    const uint16_t kMSB = 0x8000;

    KeyEvent e;
    uint32_t value = static_cast<uint32_t>(wm.wcode);

    if( (::GetAsyncKeyState(VK_LSHIFT) & kMSB) ||
        (::GetAsyncKeyState(VK_RSHIFT) & kMSB) )
    {
        value |= KeyEvent::kShift;
    }

    if( (::GetAsyncKeyState(VK_LCONTROL) & kMSB) ||
        (::GetAsyncKeyState(VK_RCONTROL) & kMSB) )
    {
        value |= KeyEvent::kControl;
    }

    if( (::GetAsyncKeyState(VK_LMENU) & kMSB) ||
        (::GetAsyncKeyState(VK_RMENU) & kMSB) )
    {
        value |= KeyEvent::kAlt;
    }

    KeyEvent::KeyValue key_value = static_cast<KeyEvent::KeyValue>(value);
    e.set_value(key_value);

    wm.rcode = 0;

    switch(wm.id)
    {
    case WM_KEYDOWN:
        {
            e.set_type(KeyEvent::kKeyDown);
            OnKeyDown(this, e);
        }
        break;
    case WM_KEYUP:
        {
            e.set_type(KeyEvent::kKeyUp);
            OnKeyUp(this, e);
        }
        break;
    case WM_CHAR:
        {
            wchar_t character = static_cast<wchar_t>(wm.wcode);
            TextEvent e;
            e.set_value(character);
            OnTextEvent(this, e);
        }
        break;
    default:
        DefWndProc(wm);
    }
    return true;
}

bool WindowBase::HandleTimer(WindowMessage & wm)
{
    assert(handle_);
    OnTime(this, wm.wcode);
    wm.rval = 0;
    return true;
}

bool WindowBase::HandleMinMaxInfo(WindowMessage & wm)
{
    assert(handle_);

    MINMAXINFO * mmi = reinterpret_cast<MINMAXINFO *>(wm.lcode);

    base::Size max_size(mmi->ptMaxSize.x, mmi->ptMaxSize.y);
    base::Point max_pos(mmi->ptMaxPosition.x, mmi->ptMaxPosition.y);
    base::Size min_track_size(mmi->ptMinTrackSize.x, mmi->ptMinTrackSize.y);
    base::Size max_track_size(mmi->ptMaxTrackSize.x, mmi->ptMaxTrackSize.y);

    WindowBoundsArgs e = { max_size, max_pos, min_track_size, max_track_size };
    QueryBounds(this, e);

    mmi->ptMaxSize.x = max_size.width();
    mmi->ptMaxSize.y = max_size.height();
    mmi->ptMaxPosition.x = max_pos.x();
    mmi->ptMaxPosition.y = max_pos.y();
    mmi->ptMinTrackSize.x = min_track_size.width();
    mmi->ptMinTrackSize.y = min_track_size.height();
    mmi->ptMaxTrackSize.x = max_track_size.width();
    mmi->ptMaxTrackSize.y = max_track_size.height();

    wm.rcode = 0;
    return true;
}

bool WindowBase::HandleNonClientSize(WindowMessage & wm)
{
    assert(handle_);

    if(wm.wcode)
    {
        base::Rect window_rect;
        int left_width = 0;
        int top_height = 0;
        int right_width = 0;
        int bottom_height = 0;

        auto ncparam = reinterpret_cast<NCCALCSIZE_PARAMS *>(wm.lcode);

        window_rect.SetLTRB(ncparam->rgrc[0].left, 
                            ncparam->rgrc[0].top,
                            ncparam->rgrc[0].right, 
                            ncparam->rgrc[0].bottom);

        DefWndProc(wm);

        left_width = ncparam->rgrc[0].left - window_rect.left();
        top_height = ncparam->rgrc[0].top - window_rect.top();
        right_width = window_rect.right() - ncparam->rgrc[0].right;
        bottom_height = window_rect.bottom() - ncparam->rgrc[0].bottom;

        MarginArgs e = { left_width, top_height, right_width, bottom_height };
        QueryMargin(this, e);

        ncparam->rgrc[0].left = window_rect.left() + left_width;
        ncparam->rgrc[0].top = window_rect.top() + top_height;
        ncparam->rgrc[0].right = window_rect.right() - right_width;
        ncparam->rgrc[0].bottom = window_rect.bottom() - bottom_height;
    }
    else
    {
        base::Rect window_rect;
        int left_width = 0;
        int top_height = 0;
        int right_width = 0;
        int bottom_height = 0;

        RECT * ncparam = reinterpret_cast<RECT *>(wm.lcode);

        window_rect.SetLTRB(ncparam->left, 
                            ncparam->top,
                            ncparam->right,
                            ncparam->bottom);

        DefWndProc(wm);

        left_width = ncparam->left - window_rect.left();
        top_height = ncparam->top - window_rect.top();
        right_width = window_rect.right() - ncparam->right;
        bottom_height = window_rect.bottom() - ncparam->bottom;

        MarginArgs e = { left_width, top_height, right_width, bottom_height };
        QueryMargin(this, e);

        ncparam->left = window_rect.left() + left_width;
        ncparam->top = window_rect.top() + top_height;
        ncparam->right = window_rect.right() - right_width;
        ncparam->bottom = window_rect.bottom() - bottom_height;
    }
   
    return true;
}

bool WindowBase::DwmDefWindowProc(WindowMessage & wm)
{
    if(!SysInfo::DWMEnabled())
        return false;

    LRESULT * lr = (LRESULT *)&wm.rval;

    if(::DwmDefWindowProc(handle_, wm.id , wm.wcode, wm.lcode, lr))
        return true;
    else
        return false;
}

}
}