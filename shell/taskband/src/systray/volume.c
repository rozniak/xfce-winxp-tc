#include <canberra.h>
#include <canberra-gtk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>
#include <wintc/sndapi.h>

#include "../intapi.h"
#include "icon.h"
#include "volume.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationVolume
{
    WinTCShextUIController __parent__;

    // UI
    //
    GtkWidget* notif_icon;

    GtkWidget* popup_volmgmt;

    GtkWidget* box_container;
    GtkWidget* check_mute;
    GtkWidget* scale_volume;

    gboolean syncing_state;

    // Sound API stuff
    //
    WinTCSndApiContext* snd_ctx;
    WinTCSndApiOutput*  snd_output;
};

//
// FORWARD DECLARATIONS
//
static void wintc_notification_volume_constructed(
    GObject* object
);

static void wintc_notification_volume_set_have_device(
    WinTCNotificationVolume* volume,
    gboolean                 have_device
);

static void on_snd_ctx_connected_changed(
    WinTCSndApiContext* ctx,
    gboolean            connected,
    gpointer            user_data
);
static void on_snd_ctx_default_output_changed(
    WinTCSndApiContext* ctx,
    gpointer            user_data
);

static void on_snd_output_muted_changed(
    WinTCSndApiOutput* output,
    gpointer           user_data
);
static void on_snd_output_volume_changed(
    WinTCSndApiOutput* output,
    gpointer           user_data
);

static gboolean on_notif_icon_button_press_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);

static void on_check_mute_toggled(
    GtkToggleButton* self,
    gpointer         user_data
);
static gboolean on_scale_volume_button_release_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);
static void on_scale_volume_value_changed(
    GtkRange* self,
    gpointer  user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationVolume,
    wintc_notification_volume,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_notification_volume_class_init(
    WinTCNotificationVolumeClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_volume_constructed;
}

static void wintc_notification_volume_init(
    WINTC_UNUSED(WinTCNotificationVolume* self)
)
{
    static gboolean css_added = FALSE;

    // Add CSS once to screen - can't do this in class because GTK may not
    // have initialized first
    //
    if (!css_added)
    {
        GtkCssProvider* css_volume_popup = gtk_css_provider_new();

        gtk_css_provider_load_from_resource(
            css_volume_popup,
            "/uk/oddmatics/wintc/taskband/volume-popup.css"
        );

        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(css_volume_popup),
            GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
        );

        css_added = TRUE;
    }

    // Other init
    //
    self->syncing_state = FALSE;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_volume_constructed(
    GObject* object
)
{
    WinTCNotificationVolume* volume = WINTC_NOTIFICATION_VOLUME(object);

    volume->notif_icon =
        wintc_ishext_ui_host_get_ext_widget(
            wintc_shext_ui_controller_get_ui_host(
                WINTC_SHEXT_UI_CONTROLLER(object)
            ),
            WINTC_NOTIFAREA_HOSTEXT_ICON,
            WINTC_TYPE_NOTIF_AREA_ICON,
            object
        );

    // Connect up to the notification icon widget
    // 
    volume->popup_volmgmt =
        wintc_dpa_create_popup(volume->notif_icon, FALSE);

    volume->box_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    volume->check_mute    = gtk_check_button_new_with_label("Mute");
    volume->scale_volume  = gtk_scale_new_with_range(
                                GTK_ORIENTATION_VERTICAL,
                                0.0f,
                                1.0f,
                                0.05f
                            );

    gtk_scale_set_draw_value(GTK_SCALE(volume->scale_volume), FALSE);
    gtk_range_set_increments(GTK_RANGE(volume->scale_volume), 0.05f, 0.1f);
    gtk_range_set_inverted(GTK_RANGE(volume->scale_volume), TRUE);

    wintc_widget_add_style_class(volume->popup_volmgmt, "wintc-volmgmt");

    gtk_box_pack_start(
        GTK_BOX(volume->box_container),
        gtk_label_new("Volume"),
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(volume->box_container),
        volume->scale_volume,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(volume->box_container),
        volume->check_mute,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(
        GTK_CONTAINER(volume->popup_volmgmt),
        volume->box_container
    );

    g_signal_connect(
        volume->check_mute,
        "toggled",
        G_CALLBACK(on_check_mute_toggled),
        object
    );
    g_signal_connect(
        volume->scale_volume,
        "button-release-event",
        G_CALLBACK(on_scale_volume_button_release_event),
        NULL
    );
    g_signal_connect(
        volume->scale_volume,
        "value-changed",
        G_CALLBACK(on_scale_volume_value_changed),
        object
    );

    g_signal_connect(
        volume->notif_icon,
        "button-press-event",
        G_CALLBACK(on_notif_icon_button_press_event),
        object
    );

    // Set initial state
    //
    wintc_notification_volume_set_have_device(volume, FALSE);

    // Establish sound API context
    //
    volume->snd_ctx = wintc_sndapi_context_new();

    g_signal_connect(
        volume->snd_ctx,
        "connected-changed",
        G_CALLBACK(on_snd_ctx_connected_changed),
        volume
    );
    g_signal_connect(
        volume->snd_ctx,
        "default-output-changed",
        G_CALLBACK(on_snd_ctx_default_output_changed),
        volume
    );

    wintc_sndapi_context_connect(volume->snd_ctx);

    // Chain up
    //
    (G_OBJECT_CLASS(
        wintc_notification_volume_parent_class
    ))->constructed(object);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_notification_volume_set_have_device(
    WinTCNotificationVolume* volume,
    gboolean                 have_device
)
{
    gtk_widget_set_sensitive(volume->popup_volmgmt, have_device);

    if (!have_device)
    {
        wintc_notif_area_icon_set_icon_name(
            WINTC_NOTIF_AREA_ICON(volume->notif_icon),
            "audio-volume-muted"
        );
    }
}

//
// CALLBACKS
//
static void on_snd_ctx_connected_changed(
    WINTC_UNUSED(WinTCSndApiContext* ctx),
    gboolean connected,
    gpointer user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    if (!connected)
    {
        wintc_notification_volume_set_have_device(volume, FALSE);
    }
}

static void on_snd_ctx_default_output_changed(
    WinTCSndApiContext* ctx,
    gpointer            user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    WinTCSndApiOutput* output =
        wintc_sndapi_context_get_default_output(ctx);

    if (volume->snd_output == output)
    {
        return;
    }

    volume->snd_output = output;

    if (volume->snd_output == NULL)
    {
        wintc_notification_volume_set_have_device(volume, FALSE);
        return;
    }

    wintc_notification_volume_set_have_device(volume, TRUE);

    g_signal_connect(
        volume->snd_output,
        "muted-changed",
        G_CALLBACK(on_snd_output_muted_changed),
        volume
    );
    g_signal_connect(
        volume->snd_output,
        "volume-changed",
        G_CALLBACK(on_snd_output_volume_changed),
        volume
    );
}

static void on_snd_output_muted_changed(
    WinTCSndApiOutput* output,
    gpointer           user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    gboolean     muted     = wintc_sndapi_output_is_muted(output);
    const gchar* icon_name = muted ?
                                 "audio-volume-muted" :
                                 "audio-volume-medium";

    volume->syncing_state = TRUE;
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(volume->check_mute),
        muted
    );
    volume->syncing_state = FALSE;

    wintc_notif_area_icon_set_icon_name(
        WINTC_NOTIF_AREA_ICON(volume->notif_icon),
        icon_name
    );
}

static void on_snd_output_volume_changed(
    WinTCSndApiOutput* output,
    gpointer           user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    gdouble new_volume = wintc_sndapi_output_get_volume(output);

    volume->syncing_state = TRUE;
    gtk_range_set_value(
        GTK_RANGE(volume->scale_volume),
        new_volume
    );
    volume->syncing_state = FALSE;
}

static gboolean on_notif_icon_button_press_event(
    WINTC_UNUSED(GtkWidget* self),
    WINTC_UNUSED(GdkEventButton* event),
    gpointer   user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    wintc_dpa_show_popup(
        volume->popup_volmgmt,
        volume->notif_icon
    );

    return TRUE;
}

static void on_check_mute_toggled(
    GtkToggleButton* self,
    gpointer         user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    if (volume->syncing_state)
    {
        return;
    }

    wintc_sndapi_output_set_muted(
        volume->snd_output,
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self))
    );
}

static gboolean on_scale_volume_button_release_event(
    WINTC_UNUSED(GtkWidget* self),
    GdkEventButton* event,
    WINTC_UNUSED(gpointer user_data)
)
{
    if (event->button != GDK_BUTTON_PRIMARY)
    {
        return FALSE;
    }

    // Ding!
    //
    ca_context* ctx = ca_gtk_context_get();

    ca_context_play(
        ctx,
        0,
        CA_PROP_EVENT_ID, "audio-volume-change",
        NULL
    );

    return FALSE;
}

static void on_scale_volume_value_changed(
    GtkRange* self,
    gpointer  user_data
)
{
    WinTCNotificationVolume* volume =
        WINTC_NOTIFICATION_VOLUME(user_data);

    if (volume->syncing_state)
    {
        return;
    }

    wintc_sndapi_output_set_volume(
        volume->snd_output,
        gtk_range_get_value(GTK_RANGE(self))
    );
}
