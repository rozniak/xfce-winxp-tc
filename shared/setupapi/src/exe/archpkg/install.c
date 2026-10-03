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
    static const gchar* s_repos[]   = { "core", "extra" };
    static const gchar* s_mirrors[] = {
        "https://geo.mirror.pkgbuild.com/$repo/os/$arch",
        "https://fastly.mirror.pkgbuild.com/$repo/os/$arch"
    };

    GError*  error     = NULL;
    GList*   list_left = NULL;
    GList*   list_pkgs = NULL;
    int      status    = EXIT_FAILURE;
    gboolean trans    = FALSE;

    alpm_errno_t   error_code;
    alpm_handle_t* handle =
        alpm_initialize("/", "/var/lib/pacman", &error_code);

    if (!handle)
    {
        g_print("ERR %s\n", alpm_strerror(error_code));
        return EXIT_FAILURE;
    }

    for (gsize i = 0; i < G_N_ELEMENTS(s_repos); i++)
    {
        alpm_db_t* db =
            alpm_register_syncdb(
                handle,
                s_repos[i],
                ALPM_SIG_USE_DEFAULT
            );

        for (gsize j = 0; j < G_N_ELEMENTS(s_mirrors); j++)
        {
            gchar* pass1 = wintc_strsubst(s_mirrors[j], "$repo", s_repos[i]);
            gchar* pass2 = wintc_strsubst(pass1, "$arch", WINTC_ARCH);

            alpm_db_add_server(db, pass2);

            g_free(pass1);
            g_free(pass2);
        }
    }

    alpm_option_set_progresscb(
        handle,
        cb_alpm_progress,
        NULL
    );

    // Init the transaction
    //
    if (alpm_trans_init(handle, ALPM_TRANS_FLAG_NEEDED) < 0)
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
            ) < 0 ||
            alpm_add_pkg(
                handle,
                package
            ) < 0
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

            continue;
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

        if (alpm_db_update(handle, alpm_dbs, 0) < 0)
        {
            goto cleanup;
        }

        for (GList* iter = list_left; iter; iter = iter->next)
        {
            const gchar* name = (gchar*) iter->data;

            alpm_pkg_t* package = NULL;

            for (alpm_list_t* iter_l = alpm_dbs; iter_l; iter_l = iter_l->next)
            {
                alpm_db_t*  alpm_db = (alpm_db_t*) iter_l->data;

                package = alpm_db_get_pkg(alpm_db, name);

                if (package && alpm_add_pkg(handle, package) < 0)
                {
                    goto cleanup;
                }
            }

            if (!package)
            {
                goto cleanup;
            }
        }
    }

    // Kick off commit
    //
    alpm_list_t* alpm_conflicts = NULL;

    if (alpm_trans_prepare(handle, &alpm_conflicts) < 0)
    {
        for (alpm_list_t* iter = alpm_conflicts; iter; iter = iter->next)
        {
            alpm_depmissing_t* problem = (alpm_depmissing_t*) iter->data;

            if (problem->causingpkg)
            {
                g_print(
                    "CONFLICT %s with %s because %s",
                    problem->target,
                    problem->depend->name,
                    problem->causingpkg
                );
            }
            else
            {
                char* sz_dep = alpm_dep_compute_string(problem->depend);

                g_print(
                    "MISSING %s requires %s",
                    problem->target,
                    sz_dep
                );

                free(sz_dep);
            }
        }

        alpm_list_free_inner(
            alpm_conflicts,
            (alpm_list_fn_free) alpm_depmissing_free
        );
        alpm_list_free(alpm_conflicts);

        goto cleanup;
    }

    trans = TRUE;

    if (!alpm_trans_get_add(handle))
    {
        status = EXIT_SUCCESS;
        goto cleanup;
    }

    if (alpm_trans_commit(handle, NULL) < 0)
    {
        goto cleanup;
    }

    status = EXIT_SUCCESS;

cleanup:
    if (status == EXIT_SUCCESS)
    {
        g_print("%s\n", "STAT 100.0");
    }
    else
    {
        g_print(
            "ERR %s\n", 
            alpm_strerror(alpm_errno(handle))
        );
    }

    if (trans)
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
    alpm_progress_t progress,
    WINTC_UNUSED(const gchar* pkg),
    gint            percent,
    size_t          howmany,
    size_t          current
)
{
    if (progress != ALPM_PROGRESS_ADD_START)
    {
        return;
    }

    gdouble current0    = current - 1;
    gdouble per_trans   = 1.0f / howmany;
    gdouble real_pct    =
        (current0 * per_trans) + (per_trans * (percent / 100.0f));

    g_print("STAT %f\n", real_pct * 100.0f);
}
