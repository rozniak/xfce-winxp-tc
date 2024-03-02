#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "../window.h"
#include "ui.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCClassicUIClass
{
    GtkWidgetClass __parent__;
};

struct _WinTCClassicUI
{
    GtkWidget __parent__;

    GtkWidget* wnd_gina;
};

//
// FORWARD DECLARATIONS
//
static gboolean wintc_classic_ui_draw(
    GtkWidget* widget,
    cairo_t*   cr
);

static void on_self_realized(
    GtkWidget* self,
    gpointer   user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCClassicUI,
    wintc_classic_ui,
    GTK_TYPE_WIDGET
)

static void wintc_classic_ui_class_init(
    WinTCClassicUIClass* klass
)
{
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    widget_class->draw = wintc_classic_ui_draw;
}

static void wintc_classic_ui_init(
    WinTCClassicUI* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);

    // Set up widgets
    //
    self->wnd_gina = wintc_gina_auth_window_new();

    // Connect to realize signal to begin when we're ready
    //
    g_signal_connect(
        self,
        "realize",
        G_CALLBACK(on_self_realized),
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static gboolean wintc_classic_ui_draw(
    WINTC_UNUSED(GtkWidget* widget),
    cairo_t* cr
)
{
    // BG color is #004E98
    // FIXME: This is the default desktop colour, should be it be defined in
    //        a shell lib? Also see shell/desktop
    //
    cairo_set_source_rgb(cr, 0.0f, 0.298f, 0.596f);
    cairo_paint(cr);

    return FALSE;
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_classic_ui_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CLASSIC_UI,
            "hexpand", TRUE,
            "vexpand", TRUE,
            NULL
        )
    );
}

//
// CALLBACKS
//
static void on_self_realized(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCClassicUI* classic_ui = WINTC_CLASSIC_UI(self);

    gtk_widget_show_all(classic_ui->wnd_gina);
}

