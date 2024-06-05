#include <dlfcn.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/category.h"
#include "../public/error.h"
#include "../public/host.h"
#include "../public/if_view.h"
#include "../public/viewitem.h"
#include "host_priv.h"

//
// CALLBACK PROTOTYPES
//
typedef gboolean (*ShextInitFunc) (
    WinTCShextHost* host
);

typedef WinTCIShextView* (*LookupViewFunc) (
    WinTCShextHost* host,
    const gchar*    path,
    GError**        error
);

//
// FORWARD DECLARATIONS
//
static void wintc_shext_host_finalize(
    GObject* object
);

WinTCIShextView* lookup_view_for_path_by_guid(
    WinTCShextHost* host,
    const gchar*    path,
    GError**        error
);
WinTCIShextView* lookup_view_for_path_by_mime(
    WinTCShextHost* host,
    const gchar*    path,
    GError**        error
);

//
// STATIC DATA
//
static LookupViewFunc s_lookup_view_funcs[] = {
    lookup_view_for_path_by_guid,
    lookup_view_for_path_by_mime,
    NULL
};

//
// GLIB OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShextHostClass
{
    GObjectClass __parent__;
};

struct _WinTCShextHost
{
    GObject __parent__;

    // View CBs
    //
    GHashTable* map_views_by_guid;
    GHashTable* map_views_by_mime;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShextHost,
    wintc_shext_host,
    G_TYPE_OBJECT
)

static void wintc_shext_host_class_init(
    WinTCShextHostClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_shext_host_finalize;
}

static void wintc_shext_host_init(
    WinTCShextHost* self
)
{
    // Set up view CB maps
    //
    self->map_views_by_guid = g_hash_table_new_full(
                                  g_str_hash,
                                  g_str_equal,
                                  g_free,
                                  NULL
                              );
    self->map_views_by_mime = g_hash_table_new_full(
                                  g_str_hash,
                                  g_str_equal,
                                  g_free,
                                  NULL
                              );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_shext_host_finalize(
    GObject* object
)
{
    WinTCShextHost* host = WINTC_SHEXT_HOST(object);

    g_hash_table_unref(host->map_views_by_guid);
    g_hash_table_unref(host->map_views_by_mime);

    (G_OBJECT_CLASS(wintc_shext_host_parent_class))->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCShextHost* wintc_shext_host_new(void)
{
    return WINTC_SHEXT_HOST(
        g_object_new(
            WINTC_TYPE_SHEXT_HOST,
            NULL
        )
    );
}

gboolean wintc_shext_host_add_toplevel_item(
    WINTC_UNUSED(WinTCShextHost*     host),
    WINTC_UNUSED(const gchar*        guid_category),
    WINTC_UNUSED(WinTCShextViewItem* view_item),
    WINTC_UNUSED(WinTCIShextView*    view),
    GError** error
)
{
    WINTC_SAFE_REF_CLEAR(error);
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

const WinTCShextCategory** wintc_shext_host_get_toplevel_categories(
    WINTC_UNUSED(WinTCShextHost* host)
)
{
    g_critical("%s Not Implemented", __func__);
    return NULL;
}

const WinTCShextViewItem** wintc_shext_host_get_toplevel_items(
    WINTC_UNUSED(WinTCShextHost* host),
    WINTC_UNUSED(const gchar*    guid_category)
)
{
    g_critical("%s Not Implemented", __func__);
    return NULL;
}

WinTCIShextView* wintc_shext_host_get_view_for_path(
    WinTCShextHost*           host,
    const WinTCShextPathInfo* path_info,
    GError**                  error
)
{
    GError*          local_error = NULL;
    WinTCIShextView* view        = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    WINTC_LOG_DEBUG(
        "shellext: view lookup for path %s",
        path_info->base_path
    );

    // Iterate through lookups 'til we get a view
    //
    for (int i = 0; s_lookup_view_funcs[i]; i++)
    {
        view =
            s_lookup_view_funcs[i](
                host,
                path_info->base_path,
                &local_error
            );

        if (view)
        {
            break;
        }
        else
        {
            // Check if there was a problem
            //
            if (local_error)
            {
                g_propagate_error(error, local_error);
                return NULL;
            }
        }
    }

    // Clear error just in case
    //
    g_clear_error(&local_error);

    // If we didn't find a view, then that is a problem
    //
    if (!view)
    {
        // FIXME: Obviously need localised string here
        //        Original string was:
        //        Cannot find '%%1!.1023ws!'. Make sure the path or Internet address is correct.
        g_set_error(
            error,
            WINTC_SHEXT_ERROR,
            WINTC_SHEXT_ERROR_HOST_NO_VIEW,
            "Cannot find '%s'. Make sure the path or Internet address is correct.",
            path_info->base_path
        );

        return NULL;
    }

    return view;
}

gboolean wintc_shext_host_load_extensions(
    WinTCShextHost* host,
    WINTC_UNUSED(WinTCShextLoadMode mode),
    WINTC_UNUSED(const gchar**      guid_filter)
)
{
    void*         dl_shext = NULL;
    const gchar*  entry    = NULL;
    GError*       error    = NULL;
    GDir*         lib_dir;
    ShextInitFunc next_init_fun;
    gchar*        next_name;

    WINTC_LOG_DEBUG("shellext: loading extensions from %s", SHEXT_LIB_DIR);

    lib_dir = g_dir_open(SHEXT_LIB_DIR, 0, &error);

    if (!lib_dir)
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    while ((entry = g_dir_read_name(lib_dir)))
    {
        next_name =
            g_build_path(G_DIR_SEPARATOR_S, SHEXT_LIB_DIR, entry, NULL);

        WINTC_LOG_DEBUG("shellext: dlopen shext %s", next_name);

        dl_shext = dlopen(next_name, RTLD_LAZY | RTLD_LOCAL);

        g_free(next_name);

        if (!dl_shext)
        {
            g_critical("shellext: failed to dlopen %s", entry);
            continue;
        }

        // Attempt to locate shext entry point and call it
        //
        next_init_fun = (ShextInitFunc) dlsym(dl_shext, "shext_init");

        if (!next_init_fun)
        {
            g_critical("shellext: unable to locate shext_init() in %s", entry);
            continue;
        }

        next_init_fun(host);
    }

    g_dir_close(lib_dir);

    return TRUE;
}

gboolean wintc_shext_host_register_property_pages_cb(
    WINTC_UNUSED(WinTCShextHost*             host),
    WINTC_UNUSED(const gchar*                mime_type),
    WINTC_UNUSED(WinTCShextPropertyPagesCtor pages_cb)
)
{
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

gboolean wintc_shext_host_register_toplevel_category(
    WINTC_UNUSED(WinTCShextHost* host),
    WINTC_UNUSED(const gchar*    guid),
    WINTC_UNUSED(const gchar*    display_name)
)
{
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

gboolean wintc_shext_host_register_view(
    WinTCShextHost*    host,
    const gchar*       guid,
    WinTCShextViewCtor factory_cb
)
{
    gchar* guid_u = g_ascii_strup(guid, -1);

    if (g_hash_table_contains(host->map_views_by_guid, guid_u))
    {
        g_critical("shellext: view guid registered twice %s", guid_u);
        g_free(guid_u);

        return FALSE;
    }

    WINTC_LOG_DEBUG("shellext: view registered: %s", guid_u);

    g_hash_table_insert(
        host->map_views_by_guid,
        guid_u,
        factory_cb
    );

    return TRUE;
}

gboolean wintc_shext_host_use_view_for_mime(
    WinTCShextHost*    host,
    const gchar*       mime_type,
    WinTCShextViewCtor factory_cb
)
{
    gchar* mime_u = g_ascii_strup(mime_type, -1);

    if (g_hash_table_contains(host->map_views_by_mime, mime_u))
    {
        g_critical("shellext: view mime registered twice %s", mime_u);
        g_free(mime_u);

        return FALSE;
    }

    WINTC_LOG_DEBUG("shellext: view registered for mime %s", mime_u);

    g_hash_table_insert(
        host->map_views_by_mime,
        mime_u,
        factory_cb
    );

    return FALSE;
}

gboolean wintc_shext_host_use_view_for_path_regex(
    WINTC_UNUSED(WinTCShextHost*    host),
    WINTC_UNUSED(GRegex*            regex),
    WINTC_UNUSED(WinTCShextViewCtor factory_cb)
)
{
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

gboolean wintc_shext_host_use_view_for_real_path(
    WINTC_UNUSED(WinTCShextHost*    host),
    WINTC_UNUSED(const gchar*       path),
    WINTC_UNUSED(WinTCShextViewCtor factory_cb)
)
{
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

//
// PRIVATE FUNCTIONS
//
WinTCIShextView* lookup_view_for_path_by_guid(
    WinTCShextHost* host,
    const gchar*    path,
    GError**        error
)
{
    static GRegex* regex_guid = NULL;

    WinTCShextViewCtor ctor;
    gchar*             guid;
    gchar*             guid_u;
    GMatchInfo*        match_info = NULL;
    WinTCIShextView*   view       = NULL;

    // Create shellext GUID regex if it hasn't already been created
    //
    if (!regex_guid)
    {
        regex_guid =
            g_regex_new(
                "^::{([A-Za-z0-9-]+)}$",
                0,
                0,
                error
            );

        if (!regex_guid)
        {
            return NULL;
        }
    }

    WINTC_LOG_DEBUG("%s", "shellext: view lookup - try guid...");

    g_regex_match(regex_guid, path, 0, &match_info);

    if (g_match_info_get_match_count(match_info))
    {
        guid   = g_match_info_fetch(match_info, 1);
        guid_u = g_ascii_strup(guid, -1);

        WINTC_LOG_DEBUG("shellext: view lookup - match guid %s", guid_u);

        ctor =
            g_hash_table_lookup(
                host->map_views_by_guid,
                guid_u
            );

        if (ctor)
        {
            view =
                ctor(
                    WINTC_SHEXT_VIEW_ASSOC_VIEW_GUID,
                    guid_u,
                    path
                );
        }
        else
        {
            WINTC_LOG_DEBUG("shellext: have no view %s", guid_u);
            // FIXME: Set error
        }

        g_free(guid);
        g_free(guid_u);
    }

    g_match_info_free(match_info);

    return view;
}

WinTCIShextView* lookup_view_for_path_by_mime(
    WinTCShextHost* host,
    const gchar*    path,
    GError**        error
)
{
    static GRegex* regex_scheme = NULL;

    WinTCShextViewCtor ctor;
    GMatchInfo*        match_info = NULL;
    gchar*             mime;
    gchar*             mime_u;
    gchar*             scheme;
    WinTCIShextView*   view       = NULL;

    if (!regex_scheme)
    {
        regex_scheme =
            g_regex_new(
                "^([A-Za-z0-9-]+)://",
                0,
                0,
                error
            );

        if (!regex_scheme)
        {
            return NULL;
        }
    }

    WINTC_LOG_DEBUG("%s", "shellext: view lookup - try scheme...");

    g_regex_match(regex_scheme, path, 0, &match_info);

    if (g_match_info_get_match_count(match_info))
    {
        scheme = g_match_info_fetch(match_info, 1);
        mime   = g_strdup_printf("x-scheme-handler/%s", scheme);
        mime_u = g_ascii_strup(mime, -1);

        WINTC_LOG_DEBUG("shellext: view lookup - match scheme %s", mime_u);

        ctor =
            g_hash_table_lookup(
                host->map_views_by_mime,
                mime_u
            );

        if (ctor)
        {
            view =
                ctor(
                    WINTC_SHEXT_VIEW_ASSOC_MIME,
                    mime_u,
                    path
                );
        }

        g_free(scheme);
        g_free(mime);
        g_free(mime_u);
    }

    g_match_info_free(match_info);

    return view;
}
