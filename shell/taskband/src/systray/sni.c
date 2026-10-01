#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menu.h>
#include <math.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "../sni-dbus.h"
#include "../snw-dbus.h"
#include "icon.h"
#include "sni.h"

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCSniIcon
{
    WinTCNotificationSni* sni;

    GtkWidget*  menu;
    GDBusProxy* proxy;
    GtkWidget*  widget;
} WinTCSniIcon;

//
// FORWARD DECLARATIONS
//
static void wintc_notification_sni_constructed(
    GObject* object
);

static void wintc_notification_sni_destroy_icon(
    WinTCNotificationSni* sni,
    WinTCSniIcon*         sni_icon
);
static GdkPixbuf* wintc_notification_sni_parse_pixmap_variant(
    GVariant* variant
);

static gboolean cb_proxy_destroy(
    gpointer user_data
);

static gboolean on_handle_register_status_notifier_item(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    const gchar*                  service,
    gpointer                      user_data
);
static gboolean on_handle_register_status_notifier_host(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    const gchar*                  service,
    gpointer                      user_data
);

static gboolean on_icon_button_release_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);

static void on_name_acquired(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);
static void on_name_lost(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);

static void on_proxy_created(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_proxy_notify_name_owner(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationSni
{
    WinTCShextUIController __parent__;

    // State
    //
    ZWinKdeStatusNotifierWatcher* dbus_snw;

    GList* list_icons;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationSni,
    wintc_notification_sni,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_notification_sni_class_init(
    WinTCNotificationSniClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_sni_constructed;
}

static void wintc_notification_sni_init(
    WinTCNotificationSni* self
)
{
    WINTC_LOG_DEBUG(
        "taskband: tray sni hosting name %s",
        "org.kde.StatusNotifierWatcher"
    );

    g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "org.kde.StatusNotifierWatcher",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        NULL,
        on_name_acquired,
        on_name_lost,
        self,
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_sni_constructed(
    WINTC_UNUSED(GObject* object)
) {}

//
// PRIVATE FUNCTIONS
//
static void wintc_notification_sni_destroy_icon(
    WinTCNotificationSni* sni,
    WinTCSniIcon*         sni_icon
)
{
    // Delete our tracking
    //
    sni->list_icons =
        g_list_delete_link(
            sni->list_icons,
            g_list_find(sni->list_icons, sni_icon)
        );

    // Queue up destruction of proxy object (so this is signal-safe)
    //
    g_idle_add(
        (GSourceFunc) cb_proxy_destroy,
        sni_icon->proxy
    );

    // Finish destroying the struct
    //
    gtk_widget_destroy(sni_icon->widget);
    g_free(sni_icon);
}

static GdkPixbuf* wintc_notification_sni_parse_pixmap_variant(
    GVariant* variant
)
{
    WINTC_LOG_DEBUG("Variant is of type: %s", g_variant_get_type_string(variant));

    GVariantIter* v_iter;

    g_variant_get(variant, "a(iiay)", &v_iter);

    // Parse out closest match to 16x16
    //
    const gint target_size = 16;

    gint       closest_diff = G_MAXINT;
    GdkPixbuf* pixbuf       = NULL;
    gint       height;
    gint       width;
    GVariant*  v_data;

    while (
        g_variant_iter_loop(
            v_iter,
            "(ii@ay)",
            &width,
            &height,
            &v_data
        )
    )
    {
        gint diff = abs(target_size - width);

        if (diff >= closest_diff)
        {
            continue;
        }

        // Read out data
        //
        gsize len;

        const guint8* pixmap_data =
            g_variant_get_fixed_array(v_data, &len, sizeof(guint8));

        if (!pixmap_data || len != (gsize)(width * height * 4))
        {
            continue;
        }

        // Create the pixbuf
        //
        guint8* pixbuf_data;
        guint   stride;

        g_clear_object(&pixbuf);

        pixbuf      = gdk_pixbuf_new(
                          GDK_COLORSPACE_RGB,
                          TRUE,
                          8,
                          width,
                          height
                      );
        pixbuf_data = gdk_pixbuf_get_pixels(pixbuf);
        stride      = gdk_pixbuf_get_rowstride(pixbuf);

        for (gint y = 0; y < height; y++)
        {
            for (gint x = 0; x < width; x++)
            {
                gint idx_pixmap = ((y * width) + x) * 4;
                gint idx_pixbuf = (y * stride) + (x * 4);

                // Pixbuf RGBA = Pixmap ARGB
                //
                pixbuf_data[idx_pixbuf + 0] = pixmap_data[idx_pixmap + 1];
                pixbuf_data[idx_pixbuf + 1] = pixmap_data[idx_pixmap + 2];
                pixbuf_data[idx_pixbuf + 2] = pixmap_data[idx_pixmap + 3];
                pixbuf_data[idx_pixbuf + 3] = pixmap_data[idx_pixmap + 0];
            }
        }

        closest_diff = diff;
    }

    g_variant_iter_free(v_iter);

    return pixbuf;
}

//
// CALLBACKS
//
static gboolean cb_proxy_destroy(
    gpointer user_data
)
{
    GDBusProxy* proxy = G_DBUS_PROXY(user_data);

    g_object_unref(proxy);

    return G_SOURCE_REMOVE;
}

static gboolean on_handle_register_status_notifier_item(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    const gchar*                  service,
    gpointer                      user_data
)
{
    WinTCNotificationSni* sni = WINTC_NOTIFICATION_SNI(user_data);

    WINTC_LOG_DEBUG("SNI: Register service call for: %s", service);

    //
    // Call in to register an item - we take in the parameter 'service' as the
    // DBus name for where to look for a /StatusNotifierItem object
    //

    if (!g_dbus_is_name(service))
    {
        g_dbus_method_invocation_return_error_literal(
            invocation,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Bus name provided is unacceptable."
        );

        return FALSE;
    }

    // Spawn the DBus connection to the icon
    //
    GDBusInterfaceInfo* info = zwin_kde_status_notifier_item_interface_info();

    g_dbus_proxy_new_for_bus(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        info,
        service,
        "/StatusNotifierItem",
        "org.kde.StatusNotifierItem",
        NULL,
        (GAsyncReadyCallback) on_proxy_created,
        sni
    );

    zwin_kde_status_notifier_watcher_complete_register_status_notifier_item(
        dbus_snw,
        invocation
    );

    return TRUE;
}

static gboolean on_handle_register_status_notifier_host(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    WINTC_UNUSED(const gchar* service),
    WINTC_UNUSED(gpointer user_data)
)
{
    // FIXME: Implement this
    //
    zwin_kde_status_notifier_watcher_complete_register_status_notifier_host(
        dbus_snw,
        invocation
    );

    return TRUE;
}

static gboolean on_icon_button_release_event(
    WINTC_UNUSED(GtkWidget* self),
    GdkEventButton* event,
    gpointer        user_data
)
{
    WinTCSniIcon* sni_icon = (WinTCSniIcon*) user_data;

    if (event->button == GDK_BUTTON_SECONDARY)
    {
        gtk_menu_popup_at_pointer(
            GTK_MENU(sni_icon->menu),
            (GdkEvent*) event
        );
    }

    return FALSE;
}

static void on_name_acquired(
    GDBusConnection* connection,
    WINTC_UNUSED(const gchar* name),
    gpointer         user_data
)
{
    WinTCNotificationSni* sni = WINTC_NOTIFICATION_SNI(user_data);

    GError* error = NULL;

    // Create the DBus host
    //
    ZWinKdeStatusNotifierWatcher* dbus_snw =
        zwin_kde_status_notifier_watcher_skeleton_new();

    zwin_kde_status_notifier_watcher_set_is_status_notifier_host_registered(
        dbus_snw,
        TRUE
    );
    zwin_kde_status_notifier_watcher_set_registered_status_notifier_items(
        dbus_snw,
        NULL
    );
    zwin_kde_status_notifier_watcher_set_protocol_version(
        dbus_snw,
        0
    );

    g_signal_connect(
        dbus_snw,
        "handle-register-status-notifier-item",
        G_CALLBACK(on_handle_register_status_notifier_item),
        sni
    );
    g_signal_connect(
        dbus_snw,
        "handle-register-status-notifier-host",
        G_CALLBACK(on_handle_register_status_notifier_host),
        sni
    );

    if (
        !g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(dbus_snw),
            connection,
            "/StatusNotifierWatcher",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        g_clear_object(&dbus_snw);
        return;
    }

    sni->dbus_snw = dbus_snw;
}

static void on_name_lost(
    WINTC_UNUSED(GDBusConnection* connection),
    WINTC_UNUSED(const gchar*     name),
    WINTC_UNUSED(gpointer         user_data)
)
{
    // FIXME: We should probably do something about this
}

static void on_proxy_created(
    WINTC_UNUSED(GObject* source_object),
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNotificationSni* sni = WINTC_NOTIFICATION_SNI(user_data);

    GError*     error = NULL;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

    if (!proxy)
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    WINTC_LOG_DEBUG(
        "SNI: successfully attached to %s",
        g_dbus_proxy_get_name(proxy)
    );

    // Spawn the struct to hold the icon widget and the DBus connection
    //
    WinTCSniIcon* sni_icon = g_new(WinTCSniIcon, 1);

    sni_icon->sni    = sni;
    sni_icon->menu   = NULL;
    sni_icon->proxy  = proxy;
    sni_icon->widget =
        wintc_ishext_ui_host_get_ext_widget(
            wintc_shext_ui_controller_get_ui_host(
                WINTC_SHEXT_UI_CONTROLLER(sni)
            ),
            WINTC_NOTIFAREA_HOSTEXT_ICON,
            WINTC_TYPE_NOTIF_AREA_ICON,
            sni
        );

    wintc_notif_area_icon_set_icon_name(
        WINTC_NOTIF_AREA_ICON(sni_icon->widget),
        "dialog-question"
    );

    sni->list_icons =
        g_list_prepend(sni->list_icons, sni_icon);

    // Set up menu
    //
    GVariant* v_menu =
        g_dbus_proxy_get_cached_property(proxy, "Menu");

    if (v_menu)
    {
        const gchar* object_path_menu = g_variant_get_string(v_menu, NULL);

        sni_icon->menu =
            GTK_WIDGET(
                dbusmenu_gtkmenu_new(
                    (gchar*) g_dbus_proxy_get_name(proxy),
                    (gchar*) object_path_menu
                )
            );

        g_object_ref_sink(sni_icon->menu);

        g_variant_unref(v_menu);
    }

    // Set up initial icon
    //
    GVariant* v_icon_name =
        g_dbus_proxy_get_cached_property(proxy, "IconName");

    if (v_icon_name)
    {
        const gchar* icon_name = NULL;

        g_variant_get(v_icon_name, "&s", &icon_name);

        wintc_notif_area_icon_set_icon_name(
            WINTC_NOTIF_AREA_ICON(sni_icon->widget),
            icon_name
        );

        g_variant_unref(v_icon_name);
    }
    else
    {
        GVariant* v_icon_pixmap =
            g_dbus_proxy_get_cached_property(proxy, "IconPixmap");

        if (v_icon_pixmap)
        {
            wintc_notif_area_icon_set_icon_pixbuf(
                WINTC_NOTIF_AREA_ICON(sni_icon->widget),
                wintc_notification_sni_parse_pixmap_variant(v_icon_pixmap)
            );

            g_variant_unref(v_icon_pixmap);
        }
    }

    // Attach signals
    //
    g_signal_connect(
        sni_icon->widget,
        "button-release-event",
        G_CALLBACK(on_icon_button_release_event),
        sni_icon
    );

    g_signal_connect(
        proxy,
        "notify::g-name-owner",
        G_CALLBACK(on_proxy_notify_name_owner),
        sni_icon
    );
}

static void on_proxy_notify_name_owner(
    WINTC_UNUSED(GObject*    self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    WinTCSniIcon* sni_icon = (WinTCSniIcon*) user_data;

    // Since the SNI DBus clients own unique names like :1.39, these can only
    // disappear so no need to check for NULL, just destroy our proxy
    //
    wintc_notification_sni_destroy_icon(
        sni_icon->sni,
        sni_icon
    );
}
