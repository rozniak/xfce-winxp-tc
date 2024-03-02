#ifndef __TOOLBAR_TASK_BUTTONS_H__
#define __TOOLBAR_TASK_BUTTONS_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCToolbarTaskButtonsClass WinTCToolbarTaskButtonsClass;
typedef struct _WinTCToolbarTaskButtons      WinTCToolbarTaskButtons;

#define WINTC_TYPE_TOOLBAR_TASK_BUTTONS            (wintc_toolbar_task_buttons_get_type())
#define WINTC_TOOLBAR_TASK_BUTTONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_TOOLBAR_TASK_BUTTONS, WinTCToolbarTaskButtons))
#define WINTC_TOOLBAR_TASK_BUTTONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_TOOLBAR_TASK_BUTTONS, WinTCToolbarTaskButtonsClass))
#define IS_WINTC_TOOLBAR_TASK_BUTTONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_TOOLBAR_TASK_BUTTONS))
#define IS_WINTC_TOOLBAR_TASK_BUTTONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_TOOLBAR_TASK_BUTTONS))
#define WINTC_TOOLBAR_TASK_BUTTONS_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_TOOLBAR_TASK_BUTTONS, WinTCToolbarTaskButtons))

GType wintc_toolbar_task_buttons_get_type(void) G_GNUC_CONST;

#endif

