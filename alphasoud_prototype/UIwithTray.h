#ifndef __UIWITHTRAY_H_
#define __UIWITHTRAY_H_

#define TOOLTIP_TEXT_LENGTH						12
#define ID_TRAY_APP_ICON						1113
#define ID_TRAY_CONTROLLER_CONTEXT_MENU_ITEM	1
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM			2
// >> 17May21
#define ID_TRAY_PEAK_CONTEXT_MENU_ITEM          3
// <<
#define WINDOW_WIDTH							500
#define WINDOW_HEIGHT							350//300
#define WM_TRAYICON ( WM_USER + 1 )
// >> 17May24
#define WM_SETCTRLID      WM_USER + 2
#define WM_SETABOUTVOLUME WM_USER + 3
#define WM_GETLIMITVOLUME WM_USER + 4
#define WM_GETBTN1STATE   WM_USER + 5
#define WM_GETBTN2STATE   WM_USER + 6
#define WM_SHOWPEAKMETER  WM_USER + 7
#define WM_BOXVSCROLL     WM_USER + 8
#define WM_MVSETCHANGED   WM_USER + 9

// <<

#endif // !__UIWITHTRAY_H_