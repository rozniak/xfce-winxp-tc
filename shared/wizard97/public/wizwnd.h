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

    GList* list_resources_pages;

    // Vfuncs
    //
    gboolean (*cancel) (
        WinTCWizard97Window* wiz_wnd,
        guint                current_page
    );
    void (*constructing_page) (
        WinTCWizard97Window* wiz_wnd,
        guint                page_num,
        GtkBuilder*          builder
    );
    gboolean (*finish) (
        WinTCWizard97Window* wiz_wnd,
        guint                current_page
    );
    guint (*get_next_page) (
        WinTCWizard97Window* wiz_wnd,
        guint                current_page
    );
    void (*help) (
        WinTCWizard97Window* wiz_wnd,
        guint                current_page
    );
};

//
// PUBLIC FUNCTIONS
//
void wintc_wizard97_window_class_setup_from_resources(
    WinTCWizard97WindowClass* wizard_class,
    const gchar*              resource_watermark,
    const gchar*              resource_header,
    ...
);

void wintc_wizard97_window_init_wizard(
    WinTCWizard97Window* wiz_wnd
);

#endif
