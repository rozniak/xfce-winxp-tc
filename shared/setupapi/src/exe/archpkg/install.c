#include <alpm.h>
#include <glib.h>
#include <stdlib.h>
#include <wintc/comgtk.h>

//
// FORWARD DECLARATIONS
//
static void cb_alpm_progress(
    gpointer        user_data,
    alpm_progress_t progress,
    const gchar*    pkg,
    gint            percent,
    size_t          howmany,
    size_t          current
);

//
// PUBLIC FUNCTIONS
//
gint wintc_setupapi_exec_install(
    gchar** packages
)
{
    GError* error     = NULL;
    GList*  list_pkgs = NULL;
    int     status    = EXIT_FAILURE;

    alpm_errno_t   error_code;
    alpm_handle_t* handle =
        alpm_initialize("/", "/var/lib/pacman", &error_code);

    if (!handle)
    {
        g_message("ERR %s", alpm_strerror(error_code));
        return EXIT_FAILURE;
    }

    alpm_option_set_progresscb(
        handle,
        cb_alpm_progress,
        NULL
    );

    // Init the transaction
    //
    if (alpm_trans_init(handle, ALPM_TRANS_FLAG_NEEDED) != 0)
    {
        goto cleanup;
    }

    // Try to load the packages from disk into the transaction
    //
    for (gchar** iter = packages; *iter; iter++)
    {
        const gchar* path = *iter;

        alpm_pkg_t* package = NULL;

        if (
            alpm_pkg_load(
                handle,
                path,
                1,
                ALPM_SIG_USE_DEFAULT,
                &package
            ) != 0 ||
            alpm_add_pkg(
                handle,
                package
            ) != 0
        )
        {
            if (package)
            {
                alpm_pkg_free(package);
            }

            goto cleanup;
        }

        // All good, track the package
        //
        list_pkgs =
            g_list_prepend(list_pkgs, package);
    }

    // Kick off commit
    //
    if (alpm_trans_commit(handle, NULL) != 0)
    {
        goto cleanup;
    }

    status = EXIT_SUCCESS;

cleanup:
    if (status != EXIT_SUCCESS)
    {
        g_message(
            "ERR %s", 
            alpm_errno(handle)
        );
    }

    if (alpm_trans_get_flags(handle) == 0)
    {
        if (list_pkgs)
        {
            for (GList* iter = list_pkgs; iter; iter = iter->next)
            {
                alpm_pkg_free((alpm_pkg_t*) iter->data);
            }

            g_list_free(list_pkgs);
        }

        alpm_trans_release(handle);
    }

    alpm_release(handle);

    return status;
}

//
// CALLBACKS
//
static void cb_alpm_progress(
    WINTC_UNUSED(gpointer user_data),
    WINTC_UNUSED(alpm_progress_t progress),
    WINTC_UNUSED(const gchar* pkg),
    gint     percent,
    size_t   howmany,
    size_t   current
)
{
    gdouble current0    = current - 1;
    gdouble per_trans   = 1.0f / howmany;
    gdouble real_pct    =
        (current0 * per_trans) + (per_trans * (percent / 100.0f));

    g_message("STAT %f", real_pct);
}
