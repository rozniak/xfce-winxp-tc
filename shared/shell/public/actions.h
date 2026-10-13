#ifndef __SHELL_ACTIONS_H__
#define __SHELL_ACTIONS_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShViewActions
{
    WinTCIShextView* view;
    GActionGroup*    action_group;
    GtkWindow*       wnd;
    GList*           targets;

    GWeakNotify cb_weak_ref;
} WinTCShViewActions;

//
// PUBLIC DEFINES
//
#define WINTC_SH_DEFINE_CB_VW_ACTIONS_CLEANUP(vwtype, map) \
    static void cb_weak_ref_actions( \
        WinTCShViewActions*  ctx, \
        GObject*             where_the_object_was \
    ) \
    { \
        ctx->action_group = NULL; \
        g_hash_table_remove(((vwtype*) ctx->view)->map, where_the_object_was); \
    }

#define WINTC_SH_INIT_VW_ACTIONS(vw, map) \
    vw->map = \
        g_hash_table_new_full( \
            g_direct_hash, \
            g_direct_equal, \
            NULL, \
            (GDestroyNotify) wintc_sh_view_actions_free \
        );

#define WINTC_SH_CREATE_VW_ACTIONS_GROUP(vw, map, actions, entries, wnd) \
    WinTCShViewActions* ctx = g_new(WinTCShViewActions, 1); \
    ctx->view         = WINTC_ISHEXT_VIEW(vw); \
    ctx->action_group = G_ACTION_GROUP(actions); \
    ctx->wnd          = wnd; \
    ctx->targets      = NULL; \
    ctx->cb_weak_ref  = (GWeakNotify) cb_weak_ref_actions; \
    g_action_map_add_action_entries( \
        G_ACTION_MAP(actions), \
        entries, \
        G_N_ELEMENTS(entries), \
        ctx \
    ); \
    g_object_weak_ref( \
        G_OBJECT(actions), \
        ctx->cb_weak_ref, \
        ctx \
    ); \
    g_hash_table_insert( \
        vw->map, \
        actions, \
        ctx \
    );

#define WINTC_SH_UPDATE_VW_ACTIONS_GROUP(ctx, items) \
    if (!ctx) \
    { \
        g_critical("%s", "shell extension given invalid actions group"); \
        return; \
    } \
    g_clear_list(&(ctx->targets), NULL); \
    ctx->targets = items;

//
// PUBLIC FUNCTIONS
//
void wintc_sh_view_actions_free(
    WinTCShViewActions* ctx
);

#endif
