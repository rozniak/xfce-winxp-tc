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
    GList*  list_left = NULL;
    GList*  list_pkgs = NULL;
    int     status    = EXIT_FAILURE;

    alpm_errno_t   error_code;
    alpm_handle_t* handle =
        alpm_initialize("/", "/var/lib/pacman", &error_code);

    if (!handle)
    {
        g_print("ERR %s\n", alpm_strerror(error_code));
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
        gchar* path = *iter;

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

            // Not an error yet, we may resolve the packages in DB
            //
            list_left =
                g_list_prepend(list_left, path);
        }

        // All good, track the package
        //
        list_pkgs =
            g_list_prepend(list_pkgs, package);
    }

    // Resolve any leftover packages from DB
    //
    if (list_left)
    {
        alpm_list_t* alpm_dbs = alpm_get_syncdbs(handle);

        for (GList* iter = list_left; iter; iter = iter->next)
        {
            const gchar* name = (gchar*) iter->data;

            for (alpm_list_t* iter_l = alpm_dbs; iter_l; iter_l = iter_l->next)
            {
                alpm_db_t*  alpm_db = (alpm_db_t*) iter_l->data;
                alpm_pkg_t* package = alpm_db_get_pkg(alpm_db, name);

                if (!package || alpm_add_pkg(handle, package) != 0)
                {
                    goto cleanup;
                }
            }
        }
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
        g_print(
            "ERR %s\n", 
            alpm_strerror(alpm_errno(handle))
        );
    }

    if (alpm_trans_get_flags(handle) == 0)
    {
        g_clear_list(&list_pkgs, NULL);
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

    g_print("STAT %f\n", real_pct);
}
