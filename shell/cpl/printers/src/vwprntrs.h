#ifndef __VWPRNTRS_H__
#define __VWPRNTRS_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCplViewPrintersClass WinTCCplViewPrintersClass;
typedef struct _WinTCCplViewPrinters      WinTCCplViewPrinters;

#define WINTC_TYPE_CPL_VIEW_PRINTERS            (wintc_cpl_view_printers_get_type())
#define WINTC_CPL_VIEW_PRINTERS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CPL_VIEW_PRINTERS, WinTCCplViewPrinters))
#define WINTC_CPL_VIEW_PRINTERS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CPL_VIEW_PRINTERS, WinTCCplViewPrintersClass))
#define IS_WINTC_CPL_VIEW_PRINTERS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CPL_VIEW_PRINTERS))
#define IS_WINTC_CPL_VIEW_PRINTERS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CPL_VIEW_PRINTERS))
#define WINTC_CPL_VIEW_PRINTERS_GET_CLASS(obj)  (G_TYPE_INSANCE_GET_CLASS((obj), WINTC_TYPE_CPL_VIEW_PRINTERS, WinTCCplViewPrinters))

GType wintc_cpl_view_printers_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_cpl_view_printers_new(void);

#endif
