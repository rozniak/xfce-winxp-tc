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

//
// PUBLIC FUNCTIONS
//
gboolean wintc_wizard97_page_get_is_exterior_page(
    WinTCWizard97Page* wiz_page
);
gboolean wintc_wizard97_page_get_is_final_page(
    WinTCWizard97Page* wiz_page
);
const gchar* wintc_wizard97_page_get_subtitle(
    WinTCWizard97Page* wiz_page
);
const gchar* wintc_wizard97_page_get_title(
    WinTCWizard97Page* wiz_page
);

#endif
