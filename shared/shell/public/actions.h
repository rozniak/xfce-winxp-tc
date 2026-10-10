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
} WinTCShViewActions;

//
// PUBLIC DEFINES
//
#define WINTC_SH_DEFINE_CB_VW_ACTIONS_CLEANUP(vwtype, vw, map) \
    static void cb_weak_ref_actions( \
        vwtype*  vw, \
        GObject* where_the_object_was \
    ) \
    { \
        g_hash_table_remove(vw->map, where_the_object_was); \
    }

#define WINTC_SH_INIT_VW_ACTIONS(vw, map) \
    vw->map = \
        g_hash_table_new_full( \
            g_direct_hash, \
            g_direct_equal, \
            NULL, \
            (GDestroyNotify) g_list_free \
        );

#define WINTC_SH_CREATE_VW_ACTIONS_GROUP(vw, map, actions, entries) \
    WinTCShViewActions* ctx = g_new(WinTCShViewActions, 1); \
    ctx->view = WINTC_ISHEXT_VIEW(vw); \
    ctx->action_group = G_ACTION_GROUP(actions); \
    g_action_map_add_action_entries( \
        G_ACTION_MAP(actions), \
        entries, \
        G_N_ELEMENTS(entries), \
        ctx \
    ); \
    g_object_weak_ref( \
        G_OBJECT(actions), \
        (GWeakNotify) cb_weak_ref_actions, \
        vw \
    ); \
    g_hash_table_insert( \
        vw->map, \
        actions, \
        NULL \
    );

#define WINTC_SH_UPDATE_VW_ACTIONS_GROUP(vw, map, actions, items) \
    if (g_hash_table_insert(vw->map, actions, items)) \
    { \
        g_critical("%s", "shell extension given invalid actions group"); \
        g_hash_table_remove(vw->map, actions); \
        return; \
    }

#endif
