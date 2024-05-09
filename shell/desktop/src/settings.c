#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/registry.h>

#include "settings.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_PIXBUF_WALLPAPER = 1
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCDesktopSettingsClass
{
    GObjectClass __parent__;
};

struct _WinTCDesktopSettings
{
    GObject __parent__;

    // State
    //
    WinTCRegistry* registry;

    // Properties
    //
    GdkPixbuf* pixbuf_wallpaper;
};

//
// FORWARD DECLARATIONS
//
static void wintc_desktop_settings_dispose(
    GObject* object
);
static void wintc_desktop_settings_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_desktop_settings_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_desktop_settings_set_wallpaper_path(
    WinTCDesktopSettings* settings,
    const gchar*          wallpaper_path
);

static void regkey_changed_desktop_cb(
    WinTCRegistry* registry,
    const gchar*   key_path,
    const gchar*   value_name,
    GVariant*      value_variant,
    gpointer       user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCDesktopSettings,
    wintc_desktop_settings,
    G_TYPE_OBJECT
)

static void wintc_desktop_settings_class_init(
    WinTCDesktopSettingsClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_desktop_settings_dispose;
    object_class->get_property = wintc_desktop_settings_get_property;
    object_class->set_property = wintc_desktop_settings_set_property;

    g_object_class_install_property(
        object_class,
        PROP_PIXBUF_WALLPAPER,
        g_param_spec_object(
            "pixbuf-wallpaper",
            "PixbufWallpaper",
            "The pixbuf that should be used as the desktop wallpaper.",
            GDK_TYPE_PIXBUF,
            G_PARAM_READWRITE
        )
    );
}

static void wintc_desktop_settings_init(
    WinTCDesktopSettings* self
)
{
    GError* error = NULL;

    // Initial registry set up
    //
    self->registry = wintc_registry_new();

    if (
        !wintc_registry_create_key(
            self->registry,
            "HKCU\\Control Panel\\Desktop",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    // Get some stuff from registry for startup
    //
    gchar* sz_wallpaper = NULL;

    if (
        !wintc_registry_get_key_value(
            self->registry,
            "HKCU\\Control Panel\\Desktop",
            "Wallpaper",
            WINTC_REG_SZ,
            &sz_wallpaper,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    if (sz_wallpaper)
    {
        wintc_desktop_settings_set_wallpaper_path(self, sz_wallpaper);
        g_free(sz_wallpaper);
    }

    // Set up watcher
    //
    wintc_registry_watch_key(
        self->registry,
        "HKCU\\Control Panel\\Desktop",
        regkey_changed_desktop_cb,
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_desktop_settings_dispose(
    GObject* object
)
{
    WinTCDesktopSettings* settings = WINTC_DESKTOP_SETTINGS(object);

    g_clear_object(&(settings->pixbuf_wallpaper));

    (G_OBJECT_CLASS(wintc_desktop_settings_parent_class))->dispose(object);
}

static void wintc_desktop_settings_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCDesktopSettings* settings = WINTC_DESKTOP_SETTINGS(object);

    switch (prop_id)
    {
        case PROP_PIXBUF_WALLPAPER:
            g_value_set_object(value, settings->pixbuf_wallpaper);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_desktop_settings_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCDesktopSettings* settings = WINTC_DESKTOP_SETTINGS(object);

    switch (prop_id)
    {
        case PROP_PIXBUF_WALLPAPER:
            g_clear_object(&(settings->pixbuf_wallpaper));
            settings->pixbuf_wallpaper = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCDesktopSettings* wintc_desktop_settings_new(void)
{
    return g_object_new(WINTC_TYPE_DESKTOP_SETTINGS, NULL);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_desktop_settings_set_wallpaper_path(
    WinTCDesktopSettings* settings,
    const gchar*          wallpaper_path
)
{
    GError*    error = NULL;
    GdkPixbuf* pixbuf_wallpaper;

    WINTC_LOG_DEBUG("desktop: setting wallpaper to %s", wallpaper_path);

    pixbuf_wallpaper =
        gdk_pixbuf_new_from_file(
            wallpaper_path,
            &error
        );

    if (!pixbuf_wallpaper)
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    g_object_set(
        settings,
        "pixbuf-wallpaper", pixbuf_wallpaper,
        NULL
    );

    g_object_unref(pixbuf_wallpaper);
}

//
// CALLBACK
//
static void regkey_changed_desktop_cb(
    WINTC_UNUSED(WinTCRegistry* registry),
    const gchar* key_path,
    const gchar* value_name,
    GVariant*    value_variant,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCDesktopSettings* settings = WINTC_DESKTOP_SETTINGS(user_data);

    WINTC_LOG_DEBUG("desktop: saw key changed %s->%s", key_path, value_name);

    if (g_strcmp0(value_name, "Wallpaper") == 0)
    {
        if (wintc_registry_get_type_for_variant(value_variant) != WINTC_REG_SZ)
        {
            WINTC_LOG_DEBUG("%s", "desktop: wallpaper setting is not REG_SZ");
            return;
        }

        wintc_desktop_settings_set_wallpaper_path(
            settings,
            g_variant_get_string(value_variant, NULL)
        );
    }
}
