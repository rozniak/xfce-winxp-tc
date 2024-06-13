#ifndef __PAGEDESK_H__
#define __PAGEDESK_H__

#include "window.h"

//
// PUBLIC FUNCTIONS
//
void wintc_cpl_desk_window_append_desktop_page(
    WinTCCplDeskWindow* wnd
);
void wintc_cpl_desk_window_load_desktop_page(
    WinTCCplDeskWindow* wnd
);
void wintc_cpl_desk_window_finalize_desktop_page(
    WinTCCplDeskWindow* wnd
);

#endif
