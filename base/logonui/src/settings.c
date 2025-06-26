#include <glib.h>
#include <wintc/comgtk.h>

#include "settings.h"

#define WINTC_INI_GROUP_LOGONUI "LogonUI"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_SESSION,
    PROP_USE_CLASSIC_LOGON,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_logonui_settings_constructed(
    GObject* object
);
static void wintc_logonui_settings_finalize(
    GObject* object
);
static void wintc_logonui_settings_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_logonui_settings_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// STATIC DATA
//
static GParamSpec* wintc_logonui_settings_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCLogonUISettings
{
    GObject __parent__;

    // Settings
    //
    gchar*   session;
    gboolean use_classic_logon;
} WinTCLogonUISettings;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCLogonUISettings,
    wintc_logonui_settings,
    G_TYPE_OBJECT
)

static void wintc_logonui_settings_class_init(
    WinTCLogonUISettingsClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_logonui_settings_constructed;
    object_class->finalize     = wintc_logonui_settings_finalize;
    object_class->get_property = wintc_logonui_settings_get_property;
    object_class->set_property = wintc_logonui_settings_set_property;

    wintc_logonui_settings_properties[PROP_SESSION] =
        g_param_spec_string(
            "session",
            "Startup session name",
            "The name of the session to start upon login.",
            "xfce",
            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
        );
    wintc_logonui_settings_properties[PROP_USE_CLASSIC_LOGON] =
        g_param_spec_boolean(
            "use-classic-logon",
            "Use classic logon interface",
            "Specifies whether to use the classic Windows logon interface.",
            FALSE, // FIXME: Depends on SKU
            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_logonui_settings_properties
    );
}

static void wintc_logonui_settings_init(
    WINTC_UNUSED(WinTCLogonUISettings* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_logonui_settings_constructed(
    GObject* object
)
{
    WinTCLogonUISettings* settings = WINTC_LOGONUI_SETTINGS(object);

    GKeyFile* config = g_key_file_new();
    GError*   error  = NULL;

    if (
        g_key_file_load_from_file(
            config,
            WINTC_CONFIG_DIR "/logonui.ini",
            G_KEY_FILE_NONE,
            &error
        )
    )
    {
        wintc_logonui_settings_load_from_key_file(
            settings,
            config
        );
    }
    else
    {
        wintc_log_error_and_clear(&error);
    }

    g_object_unref(config);

    (G_OBJECT_CLASS(wintc_logonui_settings_parent_class))
        ->constructed(object);
}

static void wintc_logonui_settings_finalize(
    GObject* object
)
{
    WinTCLogonUISettings* settings = WINTC_LOGONUI_SETTINGS(object);

    g_free(settings->session);
    settings->session = NULL;

    (G_OBJECT_CLASS(wintc_logonui_settings_parent_class))
        ->finalize(object);
}

static void wintc_logonui_settings_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCLogonUISettings* settings = WINTC_LOGONUI_SETTINGS(object);

    switch (prop_id)
    {
        case PROP_SESSION:
            g_value_set_string(value, settings->session);
            break;

        case PROP_USE_CLASSIC_LOGON:
            g_value_set_boolean(value, settings->use_classic_logon);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_logonui_settings_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCLogonUISettings* settings = WINTC_LOGONUI_SETTINGS(object);

    switch (prop_id)
    {
        case PROP_SESSION:
            wintc_logonui_settings_set_session(
                settings,
                g_value_get_string(value)
            );
            break;

        case PROP_USE_CLASSIC_LOGON:
            wintc_logonui_settings_set_use_classic_logon(
                settings,
                g_value_get_boolean(value)
            );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCLogonUISettings* wintc_logonui_settings_new(void)
{
    return WINTC_LOGONUI_SETTINGS(
        g_object_new(
            WINTC_TYPE_LOGONUI_SETTINGS,
            NULL
        )
    );
}

void wintc_logonui_settings_load_from_key_file(
    WinTCLogonUISettings* settings,
    GKeyFile*             key_file
)
{
    gchar*   session           = g_key_file_get_string(
                                     key_file,
                                     WINTC_INI_GROUP_LOGONUI,
                                     "session",
                                     NULL
                                 );
    gboolean use_classic_logon = g_key_file_get_boolean(
                                     key_file,
                                     WINTC_INI_GROUP_LOGONUI,
                                     "classic_logon",
                                     NULL
                                 );

    if (session)
    {
        wintc_logonui_settings_set_session(settings, session);
        g_free(session);
    }

    if (use_classic_logon)
    {
        wintc_logonui_settings_set_use_classic_logon(
            settings,
            use_classic_logon
        );
    }
}

const gchar* wintc_logonui_settings_get_session(
    WinTCLogonUISettings* settings
)
{
    return settings->session;
}

gboolean wintc_logonui_settings_get_use_classic_logon(
    WinTCLogonUISettings* settings
)
{
    return settings->use_classic_logon;
}

void wintc_logonui_settings_set_session(
    WinTCLogonUISettings* settings,
    const gchar*          session
)
{
    g_free(settings->session);
    settings->session = g_strdup(session);

    g_object_notify_by_pspec(
        G_OBJECT(settings),
        wintc_logonui_settings_properties[PROP_SESSION]
    );
}

void wintc_logonui_settings_set_use_classic_logon(
    WinTCLogonUISettings* settings,
    gboolean              use_classic_logon
)
{
    settings->use_classic_logon = use_classic_logon;

    g_object_notify_by_pspec(
        G_OBJECT(settings),
        wintc_logonui_settings_properties[PROP_USE_CLASSIC_LOGON]
    );
}
