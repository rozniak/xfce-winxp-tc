#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "../window.h"
#include "ui.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_classic_ui_igina_auth_ui_interface_init(
    WinTCIGinaAuthUIInterface* iface
);

static void wintc_classic_ui_constructed(
    GObject* object
);
static void wintc_classic_ui_dispose(
    GObject* object
);
static void wintc_classic_ui_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_classic_ui_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_classic_ui_draw(
    GtkWidget* widget,
    cairo_t*   cr
);

static void on_self_realized(
    GtkWidget* self,
    gpointer   user_data
);

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

    WinTCGinaLogonSession* logon_session;
    GtkWidget*             wnd_gina;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCClassicUI,
    wintc_classic_ui,
    GTK_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_IGINA_AUTH_UI,
        wintc_classic_ui_igina_auth_ui_interface_init
    )
)

static void wintc_classic_ui_igina_auth_ui_interface_init(
    WINTC_UNUSED(WinTCIGinaAuthUIInterface* iface)
) {}

static void wintc_classic_ui_class_init(
    WinTCClassicUIClass* klass
)
{
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->constructed  = wintc_classic_ui_constructed;
    object_class->dispose      = wintc_classic_ui_dispose;
    object_class->get_property = wintc_classic_ui_get_property;
    object_class->set_property = wintc_classic_ui_set_property;

    widget_class->draw = wintc_classic_ui_draw;

    g_object_class_override_property(
        object_class,
        PROP_LOGON_SESSION,
        "logon-session"
    );
}

static void wintc_classic_ui_init(
    WinTCClassicUI* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_classic_ui_constructed(
    GObject* object
)
{
    WinTCClassicUI* classic_ui = WINTC_CLASSIC_UI(object);

    if (!(classic_ui->logon_session))
    {
        g_critical("%s", "logonui: no logon session!");
        return;
    }

    // Set up widgets
    //
    classic_ui->wnd_gina =
        wintc_gina_auth_window_new(
            classic_ui->logon_session
        );

    // Connect to realize signal to begin when we're ready
    //
    g_signal_connect(
        classic_ui,
        "realize",
        G_CALLBACK(on_self_realized),
        NULL
    );
}

static void wintc_classic_ui_dispose(
    GObject* object
)
{
    WinTCClassicUI* classic_ui = WINTC_CLASSIC_UI(object);

    g_clear_object(&(classic_ui->logon_session));

    (G_OBJECT_CLASS(wintc_classic_ui_parent_class))
        ->dispose(object);
}

static void wintc_classic_ui_get_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(GValue* value),
    GParamSpec* pspec
)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_classic_ui_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCClassicUI* classic_ui = WINTC_CLASSIC_UI(object);

    switch (prop_id)
    {
        case PROP_LOGON_SESSION:
            classic_ui->logon_session = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

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
GtkWidget* wintc_classic_ui_new(
    WinTCGinaLogonSession* logon_session
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CLASSIC_UI,
            "hexpand",       TRUE,
            "vexpand",       TRUE,
            "logon-session", logon_session,
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

