#ifndef __VWZIP_H__
#define __VWZIP_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCViewZipClass WinTCViewZipClass;
typedef struct _WinTCViewZip      WinTCViewZip;

#define WINTC_TYPE_VIEW_ZIP            (wintc_view_zip_get_type())
#define WINTC_VIEW_ZIP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_VIEW_ZIP, WinTCViewZip))
#define WINTC_VIEW_ZIP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_VIEW_ZIP, WinTCViewZipClass))
#define IS_WINTC_VIEW_ZIP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_VIEW_ZIP))
#define IS_WINTC_VIEW_ZIP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_VIEW_ZIP))
#define WINTC_VIEW_ZIP_GET_CLASS(obj)  (G_TYPE_INSANCE_GET_CLASS((obj), WINTC_TYPE_VIEW_ZIP, WinTCViewZip))

GType wintc_view_zip_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_view_zip_new(
    const gchar* path,
    const gchar* rel_path
);

#endif
