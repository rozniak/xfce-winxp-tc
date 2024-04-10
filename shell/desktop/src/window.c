#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCDesktopWindowClass
{
    WinTCDpaDesktopWindowClass __parent__;
};

struct _WinTCDesktopWindow
{
    WinTCDpaDesktopWindow __parent__;
};

//
// FORWARD DECLARATION
//
static gboolean wintc_desktop_window_draw(
    GtkWidget* widget,
    cairo_t*   cr
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCDesktopWindow,
    wintc_desktop_window,
    WINTC_TYPE_DPA_DESKTOP_WINDOW
)

static void wintc_desktop_window_class_init(
    WinTCDesktopWindowClass* klass
)
{
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    widget_class->draw = wintc_desktop_window_draw;
}

static void wintc_desktop_window_init(
    WINTC_UNUSED(WinTCDesktopWindow* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static gboolean wintc_desktop_window_draw(
    WINTC_UNUSED(GtkWidget* widget),
    cairo_t* cr
)
{
    // FIXME: Just drawing default desktop background colour atm
    //
    cairo_set_source_rgb(cr, 0.0f, 0.298f, 0.596f);
    cairo_paint(cr);

    return FALSE;
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_desktop_window_new(
    WinTCDesktopApplication* app,
    GdkMonitor*              monitor
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_DESKTOP_WINDOW,
            "application", GTK_APPLICATION(app),
            "type",        GTK_WINDOW_TOPLEVEL,
            "decorated",   TRUE,
            "monitor",     monitor,
            "resizable",   FALSE,
            NULL
        )
    );
}
