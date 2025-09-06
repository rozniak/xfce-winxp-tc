#ifndef __TOOLBAR_TASK_BUTTONS_H__
#define __TOOLBAR_TASK_BUTTONS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TOOLBAR_TASK_BUTTONS (wintc_toolbar_task_buttons_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCToolbarTaskButtons,
    wintc_toolbar_task_buttons,
    WINTC,
    TOOLBAR_TASK_BUTTONS,
    WinTCShextUIController
)

#endif

