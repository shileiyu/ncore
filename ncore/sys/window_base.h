#ifndef NCORE_SYS_WINDOW_BASE_H_
#define NCORE_SYS_WINDOW_BASE_H_

#include "../utils/disallow_copy_and_assign.h"
#include "../utils/bitwise_enum.h"
#include "../utils/event_delegate.h"

#include "window.h"
#include "mouse_event.h"
#include "key_event.h"
#include "text_event.h"

namespace ncore
{
namespace sys
{

class WindowBase
    : public Window
{
public:
    enum CursorStyle
    {
        kNoCursor,
        kArrow,
        kHand,
        kIBeam,
        kWait,
        kForbid,
        kSizeNWSE,
    };

    enum ActivateParam
    {
        kInactive = 0,
        kActive = 1,
        kAcitveByClick = 2,
    };

public:
#ifdef NCORE_WINDOWS
    static bool RegWndClass();
    static bool UnregWndClass();
protected:
    static LRESULT _stdcall StaticWindowProc(HWND hWnd, 
                                             UINT uMsg, 
                                             WPARAM wParam, 
                                             LPARAM lParam);
public:
    static const wchar_t * kClassName;
#endif

public:
    WindowBase();
    ~WindowBase();

    const base::Point & loc() const;
    const base::Size & size() const;

    void GetLocalMonitorRect(base::Rect & monitor_rect,
                             base::Rect & monitor_work_rect);
#ifdef NCORE_WINDOWS
    bool SetTimer(uint32_t timer_id, uint32_t ms);
    bool KillTimer(uint32_t timer_id);

    void SetCursor(CursorStyle style);

    void ChangeCursor(CursorStyle style);

    bool ExtendFrameIntoClientArea(int left_width, 
                                   int right_width,
                                   int top_height, 
                                   int bottom_height);
#endif

    struct EventArgs
    {
    };

    struct CancelArgs
    {
        bool canceled;
    };

    struct PositionChangingArgs
    {
        base::Point & pos;
        bool denied;
    };

    struct SizeChangingArgs
    {
        base::Size & size;
        bool denied;
    };

    struct WindowBoundsArgs
    {
        base::Size & max_size;
        base::Point & max_pos;
        base::Size & min_track_size;
        base::Size & max_track_size;
    };

    struct MarginArgs
    {
        int & left;
        int & top;
        int & right;
        int & bottom;
    };

    struct PaintArgs
    {
#ifdef NCORE_WINDOWS
        HDC dc;
#endif
        base::Rect rect;
    };

#ifdef NCORE_WINDOWS
    struct MessageArgs
    {
        WindowMessage & wm;
        bool handled;
    };
#endif

#ifdef NCORE_WINDOWS
protected:
    virtual bool WndProc(WindowMessage & wm);

private:
    bool DefWndProc(WindowMessage & wm);
    bool HandleCreate(WindowMessage & wm);
    bool HandlePaint(WindowMessage & wm);
    bool HandlePositionChanges(WindowMessage & wm);
    bool HandleMouse(WindowMessage & wm);
    bool HandleKeyBoard(WindowMessage & wm);
    bool HandleTimer(WindowMessage & wm);
    bool HandleMinMaxInfo(WindowMessage & wm);
    bool HandleNonClientSize(WindowMessage & wm);
    bool DwmDefWindowProc(WindowMessage & wm);
    void DeterminePlacement();
#endif

private:
    DISALLOW_COPY_AND_ASSIGN(WindowBase);

    base::Point loc_;
    base::Size size_;
    WindowPlacement::Value placement_;

#ifdef NCORE_WINDOWS
    CursorStyle cursor_;
    bool tracking_mouse_;    
#endif

public:
    utils::EventDelegate<CancelArgs&> OnCreate;
    utils::EventDelegate<EventArgs&> OnDestroy;
    utils::EventDelegate<CancelArgs&> OnClose;
    utils::EventDelegate<PositionChangingArgs&> OnPositionChanging;
    utils::EventDelegate<EventArgs&> OnPositionChanged;
    utils::EventDelegate<SizeChangingArgs&> OnSizeChanging;
    utils::EventDelegate<EventArgs&> OnSizeChanged;
    utils::EventDelegate<EventArgs&> OnMinimized;
    utils::EventDelegate<EventArgs&> OnMaximized;
    utils::EventDelegate<EventArgs&> OnRestore;
    utils::EventDelegate<WindowBoundsArgs&> QueryBounds;
    utils::EventDelegate<MarginArgs&> QueryMargin;
    utils::EventDelegate<PaintArgs&> OnPaint;
    utils::EventDelegate<MouseEvent&> OnMouseMove;
    utils::EventDelegate<MouseEvent&> OnMouseDoubleClick;
    utils::EventDelegate<MouseEvent&> OnMouseUp;
    utils::EventDelegate<MouseEvent&> OnMouseDown;
    utils::EventDelegate<MouseEvent&> OnMouseEnter;
    utils::EventDelegate<MouseEvent&> OnMouseLeave;
    utils::EventDelegate<MouseEvent&> OnMouseWheel;
    utils::EventDelegate<ActivateParam> OnActivated;
    utils::EventDelegate<EventArgs&> OnSetFocus;
    utils::EventDelegate<EventArgs&> OnKillFocus;
    utils::EventDelegate<EventArgs&> OnEnabled;
    utils::EventDelegate<EventArgs&> OnDisable;
    utils::EventDelegate<MessageArgs&> OnMessage;
    utils::EventDelegate<TextEvent&> OnTextEvent;
    utils::EventDelegate<KeyEvent&> OnKeyUp;
    utils::EventDelegate<KeyEvent&> OnKeyDown;
    utils::EventDelegate<uint32_t> OnTime;
};

}
}

#endif