#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/exitwnd.h"
#include "../public/if_sm.h"

#define WINTC_GINA_EXITWND_SM_CALL(func) \
    { \
        GError* sm_error = NULL; \
        if (!func(dlg->sm, &sm_error)) \
        { \
            wintc_display_error_and_clear(&sm_error, GTK_WINDOW(dlg)); \
            return; \
        } \
        gtk_window_close(GTK_WINDOW(dlg)); \
    }

//
// PRIVATE ENUMS
//
enum // Class properties
{
    PROP_DIALOG_KIND = 1,
    PROP_SM,
    N_PROPERTIES
};

enum
{
    DIALOG_KIND_POWER_OPTIONS,
    DIALOG_KIND_USER_OPTIONS
};

//
// FORWARD DECLARATIONS
//
static void wintc_gina_exit_window_constructed(
    GObject* object
);
static void wintc_gina_exit_window_dispose(
    GObject* object
);
static void wintc_gina_exit_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void on_button_cancel_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_button_log_off_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_button_restart_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_button_stand_by_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_button_switch_user_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_button_turn_off_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_window_destroyed(
    GtkWidget* widget,
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCGinaExitWindow
{
    GtkApplicationWindow __parent__;

    // State
    //
    gint          dialog_kind;
    WinTCIGinaSm* sm;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCGinaExitWindow,
    wintc_gina_exit_window,
    GTK_TYPE_WINDOW
)

static void wintc_gina_exit_window_class_init(
    WinTCGinaExitWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_gina_exit_window_constructed;
    object_class->dispose      = wintc_gina_exit_window_dispose;
    object_class->set_property = wintc_gina_exit_window_set_property;

    g_object_class_install_property(
        object_class,
        PROP_DIALOG_KIND,
        g_param_spec_int(
            "dialog-kind",
            "DialogKind",
            "The kind of power options dialog to display.",
            DIALOG_KIND_POWER_OPTIONS,
            DIALOG_KIND_USER_OPTIONS,
            DIALOG_KIND_POWER_OPTIONS,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_SM,
        g_param_spec_object(
            "session-manager",
            "SessionManager",
            "The session management interface object.",
            WINTC_TYPE_IGINA_SM,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );

    // Load up styles
    //
    GtkCssProvider* css_exitwnd = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_exitwnd,
        "/uk/oddmatics/wintc/msgina/exitwnd.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_exitwnd),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
}

static void wintc_gina_exit_window_init(
    WinTCGinaExitWindow* self
)
{
    gtk_window_set_decorated(GTK_WINDOW(self), FALSE);
    gtk_window_set_title(GTK_WINDOW(self), _("Shutdown Windows"));

    wintc_widget_add_style_class(GTK_WIDGET(self), "wintc-exitwnd");

    g_signal_connect(
        GTK_WIDGET(self),
        "destroy",
        G_CALLBACK(on_window_destroyed),
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_exit_window_constructed(
    GObject* object
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(object);

    GtkBuilder* builder;
    GtkWidget*  button_restart;
    GtkWidget*  button_stand_by;
    GtkWidget*  button_turn_off;

    switch (dlg->dialog_kind)
    {
        case DIALOG_KIND_POWER_OPTIONS:
            builder =
                gtk_builder_new_from_resource(
                    "/uk/oddmatics/wintc/msgina/pwropts.ui"
                );

            gtk_builder_add_callback_symbols(
                builder,
                "on_button_restart_clicked",
                G_CALLBACK(on_button_restart_clicked),
                NULL
            );
            gtk_builder_add_callback_symbols(
                builder,
                "on_button_stand_by_clicked",
                G_CALLBACK(on_button_stand_by_clicked),
                NULL
            );
            gtk_builder_add_callback_symbols(
                builder,
                "on_button_turn_off_clicked",
                G_CALLBACK(on_button_turn_off_clicked),
                NULL
            );

            wintc_builder_get_objects(
                builder,
                "button-restart",  &button_restart,
                "button-stand-by", &button_stand_by,
                "button-turn-off", &button_turn_off,
                NULL
            );

            gtk_widget_set_sensitive(
                button_restart,
                wintc_igina_sm_can_restart(dlg->sm)
            );
            gtk_widget_set_sensitive(
                button_stand_by,
                wintc_igina_sm_can_sleep(dlg->sm)
            );
            gtk_widget_set_sensitive(
                button_turn_off,
                wintc_igina_sm_can_shut_down(dlg->sm)
            );

            break;

        case DIALOG_KIND_USER_OPTIONS:
            builder =
                gtk_builder_new_from_resource(
                    "/uk/oddmatics/wintc/msgina/usropts.ui"
                );

            gtk_builder_add_callback_symbols(
                builder,
                "on_button_log_off_clicked",
                G_CALLBACK(on_button_log_off_clicked),
                NULL
            );
            gtk_builder_add_callback_symbols(
                builder,
                "on_button_switch_user_clicked",
                G_CALLBACK(on_button_switch_user_clicked),
                NULL
            );

            break;

        default:
            g_critical(
                "msgina: exitwnd unknown dialog kind %d",
                dlg->dialog_kind
            );
            return;
    }

    gtk_builder_add_callback_symbols(
        builder,
        "on_button_cancel_clicked",
        G_CALLBACK(on_button_cancel_clicked),
        NULL
    );

    gtk_builder_connect_signals(
        builder,
        dlg
    );

    // Insert into parent
    //
    GtkWidget* main_box;

    wintc_builder_get_objects(
        builder,
        "main-box", &main_box,
        NULL
    );

    gtk_container_add(GTK_CONTAINER(dlg), main_box);

    g_object_unref(G_OBJECT(builder));
}

static void wintc_gina_exit_window_dispose(
    GObject* object
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(object);

    g_clear_object(&(dlg->sm));

    (G_OBJECT_CLASS(wintc_gina_exit_window_parent_class))->dispose(object);
}

static void wintc_gina_exit_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(object);

    switch (prop_id)
    {
        case PROP_DIALOG_KIND:
            dlg->dialog_kind = g_value_get_int(value);
            break;

        case PROP_SM:
            dlg->sm = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_exit_window_new_for_power_options(
    WinTCIGinaSm* sm
)
{
    GtkWidget* wnd =
        GTK_WIDGET(
            g_object_new(
                WINTC_TYPE_GINA_EXIT_WINDOW,
                "dialog-kind",     DIALOG_KIND_POWER_OPTIONS,
                "session-manager", sm,
                NULL
            )
        );

    GApplication* app = g_application_get_default();

    if (app && GTK_IS_APPLICATION(app))
    {
        gtk_window_set_application(
            GTK_WINDOW(wnd),
            GTK_APPLICATION(app)
        );
    }

    return wnd;
}

GtkWidget* wintc_gina_exit_window_new_for_user_options(
    WinTCIGinaSm* sm
)
{
    GtkWidget* wnd =
        GTK_WIDGET(
            g_object_new(
                WINTC_TYPE_GINA_EXIT_WINDOW,
                "dialog-kind",     DIALOG_KIND_USER_OPTIONS,
                "session-manager", sm,
                NULL
            )
        );

    GApplication* app = g_application_get_default();

    if (app && GTK_IS_APPLICATION(app))
    {
        gtk_window_set_application(
            GTK_WINDOW(wnd),
            GTK_APPLICATION(app)
        );
    }

    return wnd;
}

//
// CALLBACKS
//
static void on_button_cancel_clicked(
    GtkButton* button,
    WINTC_UNUSED(gpointer user_data)
)
{
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if (GTK_IS_WINDOW(toplevel))
    {
        gtk_window_close(GTK_WINDOW(toplevel));
    }
}

static void on_button_log_off_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(user_data);

    WINTC_GINA_EXITWND_SM_CALL(wintc_igina_sm_log_off);
}

static void on_button_restart_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(user_data);

    WINTC_GINA_EXITWND_SM_CALL(wintc_igina_sm_restart);
}

static void on_button_stand_by_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(user_data);

    WINTC_GINA_EXITWND_SM_CALL(wintc_igina_sm_sleep);
}

static void on_button_switch_user_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(user_data);

    WINTC_GINA_EXITWND_SM_CALL(wintc_igina_sm_switch_user);
}

static void on_button_turn_off_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    WinTCGinaExitWindow* dlg = WINTC_GINA_EXIT_WINDOW(user_data);

    WINTC_GINA_EXITWND_SM_CALL(wintc_igina_sm_shut_down);
}

static void on_window_destroyed(
    GtkWidget* widget,
    WINTC_UNUSED(gpointer user_data)
)
{
    GtkApplication* app = gtk_window_get_application(GTK_WINDOW(widget));

    g_application_quit(G_APPLICATION(app));
}
