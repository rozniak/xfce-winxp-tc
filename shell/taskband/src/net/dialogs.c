#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "device.h"
#include "dialogs.h"

//
// PRIVATE CONSTANTS
//
#define WLAN_KEY_DIALOG_DATA_CONFIRM  "wintc-wlan-entry-confirm"
#define WLAN_KEY_DIALOG_DATA_KEY      "wintc-wlan-entry-key"
#define WLAN_KEY_DIALOG_DATA_LABEL    "wintc-wlan-label-confirm"
#define WLAN_KEY_DIALOG_DATA_SECURITY "wintc-wlan-security"
#define WLAN_KEY_DIALOG_DATA_TITLE    "wintc-wlan-title"

#define WLAN_PROGRESS_DATA_BAR   "wintc-wlan-progress-bar"
#define WLAN_PROGRESS_DATA_LABEL "wintc-wlan-progress-label"

#define WLAN_CONNECT_ANIM_FRAMES     16
#define WLAN_CONNECT_ANIM_FRAME_RATE 6

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCNetRepair
{
    WinTCNetDevice* device;
    gchar*          connection_name;
    GCancellable*   cancellable;
    gulong          sigid_state_changed;

    GtkWidget*      dialog;
    GtkWidget*      button_close;
    GtkWidget*      label_text;

    const gchar*    step;
} WinTCNetRepair;

//
// FORWARD DECLARATIONS
//
static GtkWidget* net_dialog_new_from_resource(
    GtkWindow*   parent,
    const gchar* title,
    const gchar* resource_path,
    GtkBuilder** out_builder
);
static gboolean wlan_key_is_valid(
    const gchar*      key,
    WinTCWlanSecurity security
);
static void net_repair_update_step(
    WinTCNetRepair* repair
);

static void on_button_cancel_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_key_connect_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_ok_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_device_repair_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_device_repair_state_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_dialog_repair_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
);
static void on_entry_key_changed(
    GtkEditable* self,
    gpointer     user_data
);

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wlan_key_dialog_new(
    GtkWindow*        parent,
    const gchar*      title,
    const gchar*      ssid,
    WinTCWlanSecurity security
)
{
    GtkBuilder* builder;
    GtkWidget*  dialog;

    GtkWidget* button_cancel  = NULL;
    GtkWidget* button_connect = NULL;
    GtkWidget* entry_confirm  = NULL;
    GtkWidget* entry_key      = NULL;
    GtkWidget* label_confirm  = NULL;
    GtkWidget* label_intro    = NULL;
    GtkWidget* label_key      = NULL;

    dialog =
        net_dialog_new_from_resource(
            parent,
            title,
            "/uk/oddmatics/wintc/taskband/wlan-keydlg.ui",
            &builder
        );

    wintc_builder_get_objects(
        builder,
        "button-cancel",  &button_cancel,
        "button-connect", &button_connect,
        "entry-confirm",  &entry_confirm,
        "entry-key",      &entry_key,
        "label-confirm",  &label_confirm,
        "label-intro",    &label_intro,
        "label-key",      &label_key,
        NULL
    );

    gtk_label_set_text_with_mnemonic(
        GTK_LABEL(label_key),
        _("Network _key:")
    );
    gtk_label_set_text_with_mnemonic(
        GTK_LABEL(label_confirm),
        _("C_onfirm network key:")
    );

    gtk_label_set_mnemonic_widget(GTK_LABEL(label_key),     entry_key);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label_confirm), entry_confirm);

    gchar* text =
        g_strdup_printf(
            _("The network '%s' requires a network key (also called a WEP key or WPA key). A network key helps prevent unknown intruders from connecting to this network."),
            ssid
        );

    gtk_label_set_text(GTK_LABEL(label_intro), text);

    g_free(text);

    // XP greys out the confirmation box until something has been typed
    //
    gtk_widget_set_sensitive(label_confirm, FALSE);
    gtk_widget_set_sensitive(entry_confirm, FALSE);

    g_object_set_data(
        G_OBJECT(dialog),
        WLAN_KEY_DIALOG_DATA_KEY,
        entry_key
    );
    g_object_set_data(
        G_OBJECT(dialog),
        WLAN_KEY_DIALOG_DATA_CONFIRM,
        entry_confirm
    );
    g_object_set_data(
        G_OBJECT(dialog),
        WLAN_KEY_DIALOG_DATA_LABEL,
        label_confirm
    );
    g_object_set_data(
        G_OBJECT(dialog),
        WLAN_KEY_DIALOG_DATA_SECURITY,
        GINT_TO_POINTER(security)
    );
    g_object_set_data_full(
        G_OBJECT(dialog),
        WLAN_KEY_DIALOG_DATA_TITLE,
        g_strdup(title),
        g_free
    );

    g_object_set_data(G_OBJECT(entry_key), "button-connect", button_connect);

    g_signal_connect(
        entry_key,
        "changed",
        G_CALLBACK(on_entry_key_changed),
        dialog
    );
    g_signal_connect(
        button_connect,
        "clicked",
        G_CALLBACK(on_button_key_connect_clicked),
        dialog
    );
    g_signal_connect(
        button_cancel,
        "clicked",
        G_CALLBACK(on_button_cancel_clicked),
        dialog
    );

    gtk_widget_set_sensitive(button_connect, FALSE);

    gtk_window_set_default(GTK_WINDOW(dialog), button_connect);
    gtk_widget_grab_focus(entry_key);

    g_object_unref(builder);

    return dialog;
}

const gchar* wintc_wlan_key_dialog_get_key(
    GtkWidget* dialog
)
{
    GtkWidget* entry_key =
        g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_KEY);

    return gtk_entry_get_text(GTK_ENTRY(entry_key));
}

GtkWidget* wintc_wlan_progress_dialog_new(
    GtkWindow*   parent,
    const gchar* title,
    const gchar* ssid
)
{
    GtkBuilder* builder;
    GtkWidget*  dialog;

    GtkWidget* anim_connect  = NULL;
    GtkWidget* box_anim      = NULL;
    GtkWidget* button_cancel = NULL;
    GtkWidget* label_status  = NULL;
    GtkWidget* label_text    = NULL;
    GtkWidget* progress_bar  = NULL;

    dialog =
        net_dialog_new_from_resource(
            parent,
            title,
            "/uk/oddmatics/wintc/taskband/wlan-progdlg.ui",
            &builder
        );

    wintc_builder_get_objects(
        builder,
        "box-anim",      &box_anim,
        "button-cancel", &button_cancel,
        "label-status",  &label_status,
        "label-text",    &label_text,
        "progress-bar",  &progress_bar,
        NULL
    );

    gchar* text =
        g_strdup_printf(
            _("Please wait while Windows connects to the '%s' network."),
            ssid
        );

    gtk_label_set_text(GTK_LABEL(label_text), text);

    g_free(text);

    // The computer and access point exchanging data, from XP's resources
    //
    GError*    error  = NULL;
    GdkPixbuf* pixbuf =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/taskband/wlan-connect.png",
            &error
        );

    if (pixbuf)
    {
        anim_connect = wintc_ctl_animation_new();

        guint anim_id =
            wintc_ctl_animation_add_framesheet(
                WINTC_CTL_ANIMATION(anim_connect),
                pixbuf,
                WLAN_CONNECT_ANIM_FRAMES
            );

        gtk_widget_set_size_request(
            anim_connect,
            gdk_pixbuf_get_width(pixbuf),
            gdk_pixbuf_get_height(pixbuf) / WLAN_CONNECT_ANIM_FRAMES
        );

        gtk_box_pack_start(
            GTK_BOX(box_anim),
            anim_connect,
            TRUE,
            FALSE,
            0
        );

        gtk_widget_show(anim_connect);

        wintc_ctl_animation_play(
            WINTC_CTL_ANIMATION(anim_connect),
            anim_id,
            WLAN_CONNECT_ANIM_FRAME_RATE,
            WINTC_CTL_ANIMATION_INFINITE
        );

        g_object_unref(pixbuf);
    }
    else
    {
        wintc_log_error_and_clear(&error);
    }

    g_object_set_data(G_OBJECT(dialog), WLAN_PROGRESS_DATA_BAR,   progress_bar);
    g_object_set_data(G_OBJECT(dialog), WLAN_PROGRESS_DATA_LABEL, label_status);

    g_signal_connect(
        button_cancel,
        "clicked",
        G_CALLBACK(on_button_cancel_clicked),
        dialog
    );

    gtk_window_set_default(GTK_WINDOW(dialog), button_cancel);

    g_object_unref(builder);

    return dialog;
}

void wintc_wlan_progress_dialog_update(
    GtkWidget*      dialog,
    WinTCNetDevice* device
)
{
    GtkWidget* label_status =
        g_object_get_data(G_OBJECT(dialog), WLAN_PROGRESS_DATA_LABEL);
    GtkWidget* progress_bar =
        g_object_get_data(G_OBJECT(dialog), WLAN_PROGRESS_DATA_BAR);

    const gchar* text     = _("Connecting...");
    gdouble      fraction = 0.1f;

    switch (wintc_net_device_get_state(device))
    {
        case WINTC_NET_STATE_CONNECTING:
            text     = _("Waiting for the network...");
            fraction = 0.4f;
            break;

        case WINTC_NET_STATE_ACQUIRING:
            text     = _("Waiting for network to be ready...");
            fraction = 0.7f;
            break;

        default:
            break;
    }

    gtk_label_set_text(GTK_LABEL(label_status), text);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);
}

void wintc_wlan_progress_dialog_set_connected(
    GtkWidget* dialog
)
{
    GtkWidget* label_status =
        g_object_get_data(G_OBJECT(dialog), WLAN_PROGRESS_DATA_LABEL);
    GtkWidget* progress_bar =
        g_object_get_data(G_OBJECT(dialog), WLAN_PROGRESS_DATA_BAR);

    gtk_label_set_text(GTK_LABEL(label_status), _("Connected!"));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0f);
}

GtkWidget* wintc_wlan_unsecured_dialog_new(
    GtkWindow*   parent,
    const gchar* title,
    const gchar* ssid
)
{
    GtkBuilder* builder;
    GtkWidget*  dialog;

    GtkWidget* button_cancel  = NULL;
    GtkWidget* button_connect = NULL;
    GtkWidget* label_text     = NULL;

    dialog =
        net_dialog_new_from_resource(
            parent,
            title,
            "/uk/oddmatics/wintc/taskband/wlan-opendlg.ui",
            &builder
        );

    wintc_builder_get_objects(
        builder,
        "button-cancel",  &button_cancel,
        "button-connect", &button_connect,
        "label-text",     &label_text,
        NULL
    );

    gchar* text =
        g_strdup_printf(
            _("You are connecting to the unsecured network \"%s\". Information sent over this network is not encrypted and might be visible to other people."),
            ssid
        );

    gtk_label_set_text(GTK_LABEL(label_text), text);

    g_free(text);

    g_signal_connect(
        button_connect,
        "clicked",
        G_CALLBACK(on_button_ok_clicked),
        dialog
    );
    g_signal_connect(
        button_cancel,
        "clicked",
        G_CALLBACK(on_button_cancel_clicked),
        dialog
    );

    // XP defaults to Cancel here, the user has to deliberately opt in
    //
    gtk_window_set_default(GTK_WINDOW(dialog), button_cancel);
    gtk_widget_grab_focus(button_cancel);

    g_object_unref(builder);

    return dialog;
}

void wintc_net_repair_connection(
    WinTCNetDevice* device,
    const gchar*    connection_name,
    GtkWidget*      parent
)
{
    GtkBuilder*     builder;
    WinTCNetRepair* repair = g_new0(WinTCNetRepair, 1);

    repair->device          = g_object_ref(device);
    repair->connection_name = g_strdup(connection_name);
    repair->cancellable     = g_cancellable_new();

    gchar* title = g_strdup_printf(_("Repair %s"), connection_name);

    repair->dialog =
        net_dialog_new_from_resource(
            parent ? wintc_widget_get_toplevel_window(parent) : NULL,
            title,
            "/uk/oddmatics/wintc/taskband/net-repairdlg.ui",
            &builder
        );

    g_free(title);

    gtk_window_set_modal(GTK_WINDOW(repair->dialog), FALSE);

    wintc_builder_get_objects(
        builder,
        "button-close", &(repair->button_close),
        "label-text",   &(repair->label_text),
        NULL
    );

    g_object_unref(builder);

    g_object_add_weak_pointer(
        G_OBJECT(repair->dialog),
        (gpointer*) &(repair->dialog)
    );

    g_signal_connect(
        repair->dialog,
        "response",
        G_CALLBACK(on_dialog_repair_response),
        repair
    );
    g_signal_connect(
        repair->button_close,
        "clicked",
        G_CALLBACK(on_button_cancel_clicked),
        repair->dialog
    );

    repair->sigid_state_changed =
        g_signal_connect(
            device,
            "state-changed",
            G_CALLBACK(on_device_repair_state_changed),
            repair
        );

    net_repair_update_step(repair);

    gtk_widget_show(repair->dialog);

    wintc_net_device_repair_async(
        device,
        repair->cancellable,
        on_device_repair_finished,
        repair
    );
}

//
// PRIVATE FUNCTIONS
//
static GtkWidget* net_dialog_new_from_resource(
    GtkWindow*   parent,
    const gchar* title,
    const gchar* resource_path,
    GtkBuilder** out_builder
)
{
    GtkBuilder* builder;
    GtkWidget*  dialog;
    GtkWidget*  main_box = NULL;

    dialog =
        gtk_dialog_new_with_buttons(
            title,
            parent,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            NULL,
            NULL
        );

    // Owned dialogs don't get a taskbar button in XP
    //
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "network-wireless");
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), parent != NULL);
    gtk_window_set_position(
        GTK_WINDOW(dialog),
        parent ? GTK_WIN_POS_CENTER_ON_PARENT : GTK_WIN_POS_CENTER
    );
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    wintc_widget_add_style_class(dialog, "wintc-net-dialog");

    builder = gtk_builder_new_from_resource(resource_path);

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "main-box", &main_box,
        NULL
    );

    gtk_container_add(
        GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
        main_box
    );

    *out_builder = builder;

    return dialog;
}

static gboolean wlan_key_is_valid(
    const gchar*      key,
    WinTCWlanSecurity security
)
{
    gsize len = strlen(key);

    switch (security)
    {
        // WEP keys can also be passphrases, and SAE has no fixed length
        //
        case WINTC_WLAN_SECURITY_WEP:
        case WINTC_WLAN_SECURITY_WPA3:
            return len > 0;

        default:
            break;
    }

    // WPA(2)-PSK: 8-63 character passphrase, or 64 hex digits
    //
    if (len == 64)
    {
        for (gsize i = 0; i < len; i++)
        {
            if (!g_ascii_isxdigit(key[i]))
            {
                return FALSE;
            }
        }

        return TRUE;
    }

    return len >= 8 && len <= 63;
}

static void net_repair_update_step(
    WinTCNetRepair* repair
)
{
    const gchar*  step  = _("Renewing your IP address");
    WinTCNetState state = wintc_net_device_get_state(repair->device);

    // Until the wireless network is back there's no address to renew
    //
    if (
        wintc_net_device_get_kind(repair->device) ==
            WINTC_NET_DEVICE_KIND_WIRELESS &&
        (
            state == WINTC_NET_STATE_UNAVAILABLE  ||
            state == WINTC_NET_STATE_DISABLED     ||
            state == WINTC_NET_STATE_DISCONNECTED ||
            state == WINTC_NET_STATE_CONNECTING
        )
    )
    {
        step = _("Connecting to the wireless network");
    }

    if (step == repair->step || !(repair->dialog))
    {
        return;
    }

    repair->step = step;

    gchar* text =
        g_strdup_printf(
            "%s\n\n%s",
            _("Windows is taking the following action:"),
            step
        );

    gtk_label_set_text(GTK_LABEL(repair->label_text), text);

    g_free(text);
}

//
// CALLBACKS
//
static void on_button_cancel_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    gtk_dialog_response(GTK_DIALOG(user_data), GTK_RESPONSE_CANCEL);
}

static void on_button_key_connect_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    GtkWidget* dialog = GTK_WIDGET(user_data);

    GtkWidget* entry_confirm =
        g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_CONFIRM);
    GtkWidget* entry_key     =
        g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_KEY);
    const gchar* title       =
        g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_TITLE);

    WinTCWlanSecurity security =
        GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_SECURITY)
        );

    const gchar* key     = gtk_entry_get_text(GTK_ENTRY(entry_key));
    const gchar* confirm = gtk_entry_get_text(GTK_ENTRY(entry_confirm));

    // Same checks as XP, with the same messages
    //
    if (g_strcmp0(key, confirm) != 0)
    {
        wintc_messagebox_show(
            GTK_WINDOW(dialog),
            _("The network keys you typed do not match.\nPlease reenter the network key in the confirmation text box."),
            title,
            WINTC_BUTTONS_OK,
            WINTC_MESSAGE_ERROR
        );

        gtk_entry_set_text(GTK_ENTRY(entry_confirm), "");
        gtk_widget_grab_focus(entry_confirm);

        return;
    }

    if (!wlan_key_is_valid(key, security))
    {
        wintc_messagebox_show(
            GTK_WINDOW(dialog),
            _("The WPA key can be one of the following lengths:\n\n- Between 8 and 63 characters long\n\n- Exactly 64 characters long if you only use the characters 0-9 and A-F"),
            title,
            WINTC_BUTTONS_OK,
            WINTC_MESSAGE_ERROR
        );

        gtk_widget_grab_focus(entry_key);

        return;
    }

    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}

static void on_button_ok_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    gtk_dialog_response(GTK_DIALOG(user_data), GTK_RESPONSE_OK);
}

static void on_device_repair_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNetRepair* repair = (WinTCNetRepair*) user_data;
    GError*         error  = NULL;
    gchar*          text;

    gboolean success =
        wintc_net_device_action_finish(
            WINTC_NET_DEVICE(source_object),
            res,
            &error
        );

    g_clear_signal_handler(&(repair->sigid_state_changed), repair->device);

    if (success)
    {
        text =
            g_strdup(
                _("Windows finished repairing your connection. You can try connecting again.\n\nIf the problem persists, contact the person who manages your network.")
            );
    }
    else if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        text =
            g_strdup(
                _("Windows could not finish repairing the problem because the process was canceled.\n\nFor assistance, contact the person who manages your network.")
            );
    }
    else
    {
        text =
            g_strdup_printf(
                "%s\n%s\n\n%s",
                _("Windows could not finish repairing the problem because the following action cannot be completed:"),
                repair->step ? repair->step : _("Renewing your IP address"),
                _("For assistance, contact the person who manages your network.")
            );
    }

    if (error)
    {
        WINTC_LOG_DEBUG("net: repair failed: %s", error->message);
        g_clear_error(&error);
    }

    // The dialog stays up with the result until the user closes it
    //
    if (repair->dialog)
    {
        gtk_label_set_text(GTK_LABEL(repair->label_text), text);
        gtk_button_set_label(
            GTK_BUTTON(repair->button_close),
            wintc_lc_get_control_text(WINTC_CTLTXT_CLOSE, WINTC_PUNC_NONE)
        );

        g_object_remove_weak_pointer(
            G_OBJECT(repair->dialog),
            (gpointer*) &(repair->dialog)
        );

        // Hand over to the dialog, the response handler frees nothing now
        //
        g_signal_handlers_disconnect_by_data(repair->dialog, repair);

        g_signal_connect_swapped(
            repair->dialog,
            "response",
            G_CALLBACK(gtk_widget_destroy),
            repair->dialog
        );
    }

    g_free(text);

    g_object_unref(repair->cancellable);
    g_object_unref(repair->device);
    g_free(repair->connection_name);
    g_free(repair);
}

static void on_device_repair_state_changed(
    WINTC_UNUSED(WinTCNetDevice* device),
    gpointer user_data
)
{
    net_repair_update_step((WinTCNetRepair*) user_data);
}

static void on_dialog_repair_response(
    GtkDialog* self,
    WINTC_UNUSED(gint response_id),
    gpointer   user_data
)
{
    WinTCNetRepair* repair = (WinTCNetRepair*) user_data;

    // Still running - cancel, the result comes back via the async callback
    //
    g_cancellable_cancel(repair->cancellable);

    gtk_widget_destroy(GTK_WIDGET(self));
}

static void on_entry_key_changed(
    GtkEditable* self,
    gpointer     user_data
)
{
    GtkWidget* dialog = GTK_WIDGET(user_data);

    GtkWidget* button_connect =
        g_object_get_data(G_OBJECT(self), "button-connect");
    GtkWidget* entry_confirm  =
        g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_CONFIRM);
    GtkWidget* label_confirm  =
        g_object_get_data(G_OBJECT(dialog), WLAN_KEY_DIALOG_DATA_LABEL);

    gboolean has_key = *(gtk_entry_get_text(GTK_ENTRY(self))) != '\0';

    gtk_widget_set_sensitive(button_connect, has_key);
    gtk_widget_set_sensitive(entry_confirm,  has_key);
    gtk_widget_set_sensitive(label_confirm,  has_key);
}
