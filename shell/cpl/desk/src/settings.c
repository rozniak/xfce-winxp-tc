#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/registry.h>
#include <wintc/syscfg.h>

#include "settings.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_SETTINGS_CHANGED = 1
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplDeskSettingsClass
{
    GObjectClass __parent__;
};

struct _WinTCCplDeskSettings
{
    GObject __parent__;

    // Properties
    //
    gchar*              wallpaper_path;
    WinTCWallpaperStyle wallpaper_style;

    // State
    //
    gboolean       changed;
    WinTCRegistry* registry;
};

//
// FORWARD DELCARATIONS
//
static void wintc_cpl_desk_settings_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_cpl_desk_settings_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_cpl_desk_settings_bump_changed(
    WinTCCplDeskSettings* settings
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCplDeskSettings,
    wintc_cpl_desk_settings,
    G_TYPE_OBJECT
)

static void wintc_cpl_desk_settings_class_init(
    WinTCCplDeskSettingsClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_cpl_desk_settings_get_property;
    object_class->set_property = wintc_cpl_desk_settings_set_property;

    g_object_class_install_property(
        object_class,
        PROP_SETTINGS_CHANGED,
        g_param_spec_boolean(
            "settings-changed",
            "SettingsChanged",
            "The state of whether there are unsaved changes to settings.",
            FALSE,
            G_PARAM_READWRITE
        )
    );
}

static void wintc_cpl_desk_settings_init(
    WinTCCplDeskSettings* self
)
{
    self->registry = wintc_registry_new();
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_desk_settings_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCCplDeskSettings* settings = WINTC_CPL_DESK_SETTINGS(object);

    switch (prop_id)
    {
        case PROP_SETTINGS_CHANGED:
            g_value_set_boolean(value, settings->changed);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_cpl_desk_settings_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCCplDeskSettings* settings = WINTC_CPL_DESK_SETTINGS(object);

    switch (prop_id)
    {
        case PROP_SETTINGS_CHANGED:
            settings->changed = g_value_get_boolean(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCCplDeskSettings* wintc_cpl_desk_settings_new(void)
{
    return g_object_new(WINTC_TYPE_CPL_DESK_SETTINGS, NULL);
}

gboolean wintc_cpl_desk_settings_apply(
    WinTCCplDeskSettings* settings,
    GError**              error
)
{
    if (
        !wintc_registry_set_key_value(
            settings->registry,
            "HKCU\\Control Panel\\Desktop",
            "Wallpaper",
            WINTC_REG_SZ,
            &(settings->wallpaper_path),
            FALSE,
            error
        )
    )
    {
        return FALSE;
    }

    if (
        !wintc_registry_set_key_value(
            settings->registry,
            "HKCU\\Control Panel\\Desktop",
            "WallpaperStyle",
            WINTC_REG_DWORD,
            &(settings->wallpaper_style),
            FALSE,
            error
        )
    )
    {
        return FALSE;
    }

    g_object_set(
        settings,
        "settings-changed", FALSE,
        NULL
    );

    return TRUE;
}

gboolean wintc_cpl_desk_settings_load(
    WinTCCplDeskSettings* settings,
    GError**              error
)
{
    gint   dw_style     = 0;
    gchar* sz_wallpaper = NULL;

    // FIXME: Same note is in shell/desktop - could do with an API for loading
    //        in registry values from an array or something
    //
    if (
        !wintc_registry_get_key_value(
            settings->registry,
            "HKCU\\Control Panel\\Desktop",
            "Wallpaper",
            WINTC_REG_SZ,
            &sz_wallpaper,
            error
        )
    )
    {
        return FALSE;
    }

    if (
        !wintc_registry_get_key_value(
            settings->registry,
            "HKCU\\Control Panel\\Desktop",
            "WallpaperStyle",
            WINTC_REG_DWORD,
            &dw_style,
            error
        )
    )
    {
        g_free(sz_wallpaper);
        return FALSE;
    }

    // Bin old settings
    //
    g_free(g_steal_pointer(&(settings->wallpaper_path)));

    // Validate these key values
    //
    if (
        dw_style < WINTC_WALLPAPER_STYLE_CENTER ||
        dw_style > WINTC_WALLPAPER_STYLE_STRETCH
    )
    {
        dw_style = WINTC_WALLPAPER_STYLE_CENTER;
    }

    // Load in new settings
    //
    settings->wallpaper_path  = sz_wallpaper;
    settings->wallpaper_style = dw_style;

    g_object_set(
        settings,
        "settings-changed", FALSE,
        NULL
    );

    return TRUE;
}

const gchar* wintc_cpl_desk_settings_get_wallpaper(
    WinTCCplDeskSettings* settings
)
{
    return settings->wallpaper_path;
}

WinTCWallpaperStyle wintc_cpl_desk_settings_get_wallpaper_style(
    WinTCCplDeskSettings* settings
)
{
    return settings->wallpaper_style;
}

void wintc_cpl_desk_settings_set_wallpaper(
    WinTCCplDeskSettings* settings,
    const gchar*          path
)
{
    g_free(g_steal_pointer(&(settings->wallpaper_path)));

    settings->wallpaper_path = g_strdup(path);

    wintc_cpl_desk_settings_bump_changed(settings);
}

void wintc_cpl_desk_settings_set_wallpaper_style(
    WinTCCplDeskSettings* settings,
    WinTCWallpaperStyle   style
)
{
    settings->wallpaper_style = style;

    wintc_cpl_desk_settings_bump_changed(settings);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_cpl_desk_settings_bump_changed(
    WinTCCplDeskSettings* settings
)
{
    g_object_set(
        settings,
        "settings-changed", TRUE,
        NULL
    );
}
