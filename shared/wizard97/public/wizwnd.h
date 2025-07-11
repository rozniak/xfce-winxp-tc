#ifndef __WIZARD97_WIZWND_H__
#define __WIZARD97_WIZWND_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WIZARD97_WINDOW (wintc_wizard97_window_get_type())

G_DECLARE_DERIVABLE_TYPE(
    WinTCWizard97Window,
    wintc_wizard97_window,
    WINTC,
    WIZARD97_WINDOW,
    GtkWindow
)

struct _WinTCWizard97WindowClass
{
    GtkWindowClass __parent__;

    // Wizard stuff
    //
    gchar* resource_watermark;
    gchar* resource_header;

    gchar* resource_ext_page;
    GList* list_resources_int_pages;
};

//
// PUBLIC FUNCTIONS
//
void wintc_wizard97_window_class_setup_from_resources(
    WinTCWizard97WindowClass* wizard_class,
    const gchar*              resource_watermark,
    const gchar*              resource_header,
    const gchar*              resource_ext_page,
    ...
);

void wintc_wizard97_window_init_wizard(
    WinTCWizard97Window* wiz_wnd
);

#endif
