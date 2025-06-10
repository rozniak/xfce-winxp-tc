#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <lightdm.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "userlist.h"


//
// PRIVATE ENUMS
//
enum
{
    PROP_LOGON_SESSION = 1,
    N_PROPERTIES
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWelcomeUserListClass
{
    GtkContainerClass __parent__;
};

struct _WinTCWelcomeUserList
{
    GtkContainer __parent__;

    // UI
    //
    WinTCGinaLogonSession* logon_session;

    // Graphic resources
    //

};

//
// FORWARD DECLARATIONS
//
static void wintc_welcome_user_list_set_property(
    GObject*      gobject,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);
static void wintc_welcome_user_list_get_property(
    GObject*    gobject,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_welcome_user_list_finalize(
    GObject* gobject
);



//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWelcomeUserList,
    wintc_welcome_user_list,
    GTK_TYPE_CONTAINER
)

static void wintc_welcome_user_list_class_init(
    WinTCWelcomeUserListClass* klass
)
{
    GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
    GtkWidgetClass*    widget_class    = GTK_WIDGET_CLASS(klass);
    GObjectClass*      object_class    = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_welcome_user_list_get_property;
    object_class->set_property = wintc_welcome_user_list_set_property;
    object_class->finalize     = wintc_welcome_user_list_finalize;

    g_object_class_install_property(
        object_class,
        PROP_LOGON_SESSION,
        g_param_spec_object(
            "logon-session",
            "LogonSession",
            "The GINA logon session instance.",
            WINTC_TYPE_GINA_LOGON_SESSION,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_welcome_user_list_init(
    WinTCWelcomeUserList* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), TRUE);

    // Set up image resources
    //
   
    // Set up widgets
    //


    // Add style classes
    //

    // Retrieve users
    //
    // self->users =
    //     lightdm_user_list_get_users(
    //         lightdm_user_list_get_instance()
    //     );

}

//
// CLASS VIRTUAL METHODS
//
static void wintc_welcome_user_list_finalize(
    GObject* gobject
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);


    (G_OBJECT_CLASS(wintc_welcome_user_list_parent_class))->finalize(gobject);
}

static void wintc_welcome_user_list_get_property(
    GObject*    gobject,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    switch (prop_id)
    {

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}

static void wintc_welcome_user_list_set_property(
    GObject*      gobject,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(gobject);

    switch (prop_id)
    {
        case PROP_LOGON_SESSION:
           user_list->logon_session =
               WINTC_GINA_LOGON_SESSION(g_value_get_object(value));

           g_signal_connect(
               user_list->logon_session,
               "attempt-complete",
               G_CALLBACK(on_logon_session_attempt_complete),
               user_list
           );

           break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}








//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_user_list_new(
    WinTCGinaLogonSession* logon_session
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_WELCOME_USER_LIST,
            "logon-session", logon_session,
            "hexpand",       TRUE,
            "vexpand",       TRUE,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//


//
// CALLBACKS
//

static void on_logon_session_attempt_complete(
    WINTC_UNUSED(WinTCGinaLogonSession* logon_session),
    WINTC_UNUSED(WinTCGinaResponse response),
    gpointer          user_data
)
{
    WinTCWelcomeUserList* user_list = WINTC_WELCOME_USER_LIST(user_data);

    // Reset the UI state after any logon attempt
    //
}

