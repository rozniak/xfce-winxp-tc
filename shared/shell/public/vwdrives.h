#ifndef __SHELL_VWDRIVES_H__
#define __SHELL_VWDRIVES_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShViewDrivesClass WinTCShViewDrivesClass;
typedef struct _WinTCShViewDrives      WinTCShViewDrives;

#define WINTC_TYPE_SH_VIEW_DRIVES            (wintc_sh_view_drives_get_type())
#define WINTC_SH_VIEW_DRIVES(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_VIEW_DRIVES, WinTCShViewDrives))
#define WINTC_SH_VIEW_DRIVES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_VIEW_DRIVES, WinTCShViewDrivesClass))
#define IS_WINTC_SH_VIEW_DRIVES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_VIEW_DRIVES))
#define IS_WINTC_SH_VIEW_DRIVES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_VIEW_DRIVES))
#define WINTC_SH_VIEW_DRIVES_GET_CLASS(obj)  (G_TYPE_INSANCE_GET_CLASS((obj), WINTC_TYPE_SH_VIEW_DRIVES, WinTCShViewDrives))

GType wintc_sh_view_drives_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_drives_new(void);

#endif
