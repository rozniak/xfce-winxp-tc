#include <dlfcn.h>
#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>

#include "../public/ext.h"

#define S_TOOLBAR_GROUP_NAME      "ShToolbar"
#define S_TOOLBAR_KEY_LIB         "Lib"
#define S_TOOLBAR_KEY_NAME        "Name"
#define S_TOOLBAR_KEY_USE_BROWSER "UseInBrowser"
#define S_TOOLBAR_KEY_USE_CHROME  "UseInChrome"

//
// PUBLIC FUNCTIONS
//
WinTCShextExtFunc wintc_shext_ext_get_api(
    const gchar* lib_path
)
{
    void*             dl_shext = NULL;
    WinTCShextExtFunc fn_shext = NULL;

    dl_shext = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);

    if (dl_shext)
    {
        fn_shext = (WinTCShextExtFunc) dlsym(dl_shext, "wintc_shext_init");

        if (!fn_shext)
        {
            g_warning(
                "shellext: unable to find wintc_shext_init in %s",
                lib_path
            );
        }
    }
    else
    {
        g_warning("shellext: unable to dlopen %s", lib_path);
    }

    return fn_shext;
}

GList* wintc_shext_ext_get_toolbar_apis(
    WinTCShextShellWindowKind shwnd_kind
)
{
    GError* error = NULL;
    GList*  list_inis;
    GList*  ret   = NULL;

    list_inis =
        wintc_sh_fs_get_names_as_list(
            WINTC_ASSETS_DIR "/toolbar",
            TRUE,
            G_FILE_TEST_EXISTS,
            FALSE,
            &error
        );

    for (GList* iter = list_inis; iter; iter = iter->next)
    {
        GKeyFile* ini       = g_key_file_new();
        gchar*    lib_name  = NULL;
        gchar*    lib_path  = NULL;
        gchar*    lib_fpath = NULL;

        if (
            !g_key_file_load_from_file(
                ini,
                (gchar*) iter->data,
                G_KEY_FILE_NONE,
                &error
            ) ||
            !g_key_file_get_boolean(
                ini,
                S_TOOLBAR_GROUP_NAME,
                shwnd_kind == WINTC_SHEXT_SHWND_KIND_BROWSER ?
                    S_TOOLBAR_KEY_USE_BROWSER : S_TOOLBAR_KEY_USE_CHROME,
                &error
            )
        )
        {
            goto cleanup_ini;
        }

        lib_name =
            g_key_file_get_string(
                ini,
                S_TOOLBAR_GROUP_NAME,
                S_TOOLBAR_KEY_NAME,
                NULL
            );
        lib_path =
            g_key_file_get_string(
                ini,
                S_TOOLBAR_GROUP_NAME,
                S_TOOLBAR_KEY_LIB,
                NULL
            );

        if (!lib_name || !lib_path)
        {
            goto cleanup_ini;
        }

        lib_fpath =
            g_build_path(
                G_DIR_SEPARATOR_S,
                WINTC_SYSTEM_LIB_DIR,
                "wintc",
                "toolbar",
                lib_path,
                NULL
            );

        // Attempt to load the toolbar - it will call get_ext_widget()
        //
        WinTCShextExtFunc fn_shext =
            wintc_shext_ext_get_api(lib_fpath);

        if (!fn_shext)
        {
            goto cleanup_ini;
        }

        // Toolbar loaded, store it now
        //
        WinTCShextToolbarInfo* toolbar_info =
            g_new(WinTCShextToolbarInfo, 1);

        toolbar_info->fn_shext = fn_shext;
        toolbar_info->name     = g_steal_pointer(&lib_name);
        toolbar_info->soname   = g_steal_pointer(&lib_path);
        toolbar_info->toolbar  = NULL;

        ret =
            g_list_prepend(ret, toolbar_info);

cleanup_ini:
        if (error)
        {
            wintc_log_error_and_clear(&error);
        }

        g_free(lib_name);
        g_free(lib_path);
        g_free(lib_fpath);
        g_key_file_unref(ini);
    }

    g_clear_list(&list_inis, (GDestroyNotify) g_free);

    return g_list_reverse(ret);
}

void wintc_shext_toolbar_info_free(
    WinTCShextToolbarInfo* toolbar_info
)
{
    if (!toolbar_info)
    {
        return;
    }

    g_free(toolbar_info->soname);
    g_free(toolbar_info->name);
    g_free(toolbar_info);
}
