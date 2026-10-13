#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCmdApplicationClass WinTCCmdApplicationClass;
typedef struct _WinTCCmdApplication      WinTCCmdApplication;

#define WINTC_TYPE_HELLO_APPLICATION            (wintc_cmd_application_get_type())
#define WINTC_CMD_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_HELLO_APPLICATION, WinTCCmdApplication))
#define WINTC_CMD_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_HELLO_APPLICATION, WinTCCmdApplicationClass))
#define IS_WINTC_CMD_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_HELLO_APPLICATION))
#define IS_WINTC_CMD_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_HELLO_APPLICATION))
#define WINTC_CMD_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_HELLO_APPLICATION, WinTCCmdApplicationClass))

GType wintc_cmd_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCCmdApplication* wintc_cmd_application_new(void);

#endif
