#include <glib.h>
#include <gtk/gtk.h>

#include "../public/actions.h"

//
// PUBLIC FUNCTIONS
//
void wintc_sh_view_actions_free(
    WinTCShViewActions* ctx
)
{
    g_clear_list(&(ctx->targets), NULL);

    if (ctx->action_group)
    {
        g_object_weak_unref(
            G_OBJECT(ctx->action_group),
            ctx->cb_weak_ref,
            ctx
        );
    }

    g_free(ctx);
}
