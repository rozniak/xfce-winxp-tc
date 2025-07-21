#ifndef __WIZARD97_WIZPAGE_H__
#define __WIZARD97_WIZPAGE_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WIZARD97_PAGE (wintc_wizard97_page_get_type())

G_DECLARE_DERIVABLE_TYPE(
    WinTCWizard97Page,
    wintc_wizard97_page,
    WINTC,
    WIZARD97_PAGE,
    GtkBox
)

struct _WinTCWizard97PageClass
{
    GtkBoxClass __parent__;
};

#endif
