#include <garcon/garcon.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "mfu.h"
#include "util.h"

#define MFU_WINNERS_FILENAME "mfu-winners"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_MFU_UPDATED = 0,
    N_SIGNALS
};

//
// PRIVATE STRUCTS
//
typedef struct _WinTCStartMfuEntry
{
    GarconMenuItem* garcon_item;
    gint            score;
} WinTCStartMfuEntry;

//
// FORWARD DECLARATIONS
//
static void wintc_start_mfu_tracker_dispose(
    GObject* object
);

static void wintc_start_mfu_tracker_bubble_entry(
    WinTCStartMfuTracker* mfu_tracker,
    GList*                li_entry
);
static void wintc_start_mfu_tracker_load_winners(
    WinTCStartMfuTracker* mfu_tracker
);
static void wintc_start_mfu_tracker_save_winners(
    WinTCStartMfuTracker* mfu_tracker
);

//
// STATIC DATA
//
static gint wintc_start_mfu_tracker_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCStartMfuTracker
{
    GObject __parent__;

    gboolean    loading_winners;

    GList*      list_scores;
    GHashTable* map_cmdline_to_list_item;
    GarconMenu* menu_all;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCStartMfuTracker,
    wintc_start_mfu_tracker,
    G_TYPE_OBJECT
)

static void wintc_start_mfu_tracker_class_init(
    WinTCStartMfuTrackerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_start_mfu_tracker_dispose;

    wintc_start_mfu_tracker_signals[SIGNAL_MFU_UPDATED] =
        g_signal_new(
            "mfu-updated",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

static void wintc_start_mfu_tracker_init(
    WinTCStartMfuTracker* self
)
{
    GError* error = NULL;

    // Internally, we track pretty much every possible item that could be
    // launched
    //
    self->menu_all =
        garcon_menu_new_for_path(
            WINTC_ASSETS_DIR "/shell-res/all.menu"
        );

    if (!garcon_menu_load(self->menu_all, NULL, &error))
    {
        wintc_display_error_and_clear(&error, NULL);
        return;
    }

    // Populate collections with the garcon items
    //
    // --> The list tracks the score of each garcon item, as programs are
    //     launched, their score increases and they are 'bubbled up' in the
    //     score list
    // --> The map allows translation of cmdline to the item in the
    //     list
    //
    GList* elements = garcon_menu_get_elements(self->menu_all);

    self->map_cmdline_to_list_item =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            (GDestroyNotify) g_free,
            NULL
        );

    for (GList* iter = elements; iter; iter = iter->next)
    {
        WinTCStartMfuEntry* entry = g_new0(WinTCStartMfuEntry, 1);

        entry->garcon_item = GARCON_MENU_ITEM(iter->data);

        self->list_scores =
            g_list_prepend(
                self->list_scores,
                entry
            );

        g_hash_table_insert(
            self->map_cmdline_to_list_item,
            garcon_menu_item_get_command_expanded(entry->garcon_item),
            self->list_scores
        );
    }

    self->list_scores = g_list_reverse(self->list_scores);

    g_list_free(elements);

    // Load the previous winners (if applicable)
    //
    wintc_start_mfu_tracker_load_winners(self);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_start_mfu_tracker_dispose(
    GObject* object
)
{
    WinTCStartMfuTracker* mfu_tracker = WINTC_START_MFU_TRACKER(object);

    g_hash_table_destroy(
        g_steal_pointer(&(mfu_tracker->map_cmdline_to_list_item))
    );
    g_clear_list(
        &(mfu_tracker->list_scores),
        (GDestroyNotify) g_free
    );
    g_clear_object(&(mfu_tracker->menu_all));

    (G_OBJECT_CLASS(wintc_start_mfu_tracker_parent_class))
        ->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCStartMfuTracker* wintc_start_mfu_tracker_get_default(void)
{
    static WinTCStartMfuTracker* singleton_tracker = NULL;

    if (!singleton_tracker)
    {
        singleton_tracker =
            WINTC_START_MFU_TRACKER(
                g_object_new(
                    WINTC_TYPE_START_MFU_TRACKER,
                    NULL
                )
            );
    }

    g_object_ref(singleton_tracker);

    return singleton_tracker;
}

void wintc_start_mfu_tracker_bump_cmdline(
    WinTCStartMfuTracker* mfu_tracker,
    const gchar*          cmdline
)
{
    GList* li_entry =
        g_hash_table_lookup(
            mfu_tracker->map_cmdline_to_list_item,
            cmdline
        );

    // For now, just find the Garcon item and report on it
    //
    if (li_entry)
    {
        WinTCStartMfuEntry* entry = (WinTCStartMfuEntry*) li_entry->data;

        entry->score++;

        wintc_start_mfu_tracker_bubble_entry(
            mfu_tracker,
            li_entry
        );

        WINTC_LOG_DEBUG(
            "mfu: bumped %s",
            garcon_menu_item_get_name(entry->garcon_item)
        );
        WINTC_LOG_DEBUG(
            "mfu: new pos: %d",
            g_list_position(
                mfu_tracker->list_scores,
                li_entry
            )
        );
    }
    else
    {
        WINTC_LOG_DEBUG("mfu: unknown cmdline '%s'", cmdline);
    }
}

GList* wintc_start_mfu_tracker_get_mfu_list(
    WinTCStartMfuTracker* mfu_tracker
)
{
    GList* list_mfu = NULL;

    // We return the top 6 items here, if possible
    //
    gint   i;
    GList* iter;

    for (
        iter = mfu_tracker->list_scores, i = 0;
        iter && i < 6;
        iter = iter->next, i++
    )
    {
        list_mfu =
            g_list_prepend(
                list_mfu,
                ((WinTCStartMfuEntry*) iter->data)->garcon_item
            );
    }

    return g_list_reverse(list_mfu);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_start_mfu_tracker_bubble_entry(
    WinTCStartMfuTracker* mfu_tracker,
    GList*                li_entry
)
{
    WinTCStartMfuEntry* this_entry = (WinTCStartMfuEntry*) li_entry->data;

    // Very crude method of bubbling up the item based on score, we check
    //
    GList* iter;

    for (iter = li_entry->prev; iter; iter = iter->prev)
    {
        WinTCStartMfuEntry* cmp_entry = (WinTCStartMfuEntry*) iter->data;

        if (this_entry->score < cmp_entry->score)
        {
            break;
        }
    }

    // Did we move at all?
    //
    if (iter == li_entry->prev)
    {
        return;
    }

    // Reinsert the list item in the right place
    //
    mfu_tracker->list_scores =
        g_list_remove_link(
            mfu_tracker->list_scores,
            li_entry
        );

    if (!iter)
    {
        mfu_tracker->list_scores =
            g_list_insert_before_link(
                mfu_tracker->list_scores,
                mfu_tracker->list_scores,
                li_entry
            );
    }
    else
    {
        mfu_tracker->list_scores =
            g_list_insert_before_link(
                mfu_tracker->list_scores,
                iter->next,
                li_entry
            );
    }

    g_signal_emit(
        mfu_tracker,
        wintc_start_mfu_tracker_signals[SIGNAL_MFU_UPDATED],
        0
    );

    if (!(mfu_tracker->loading_winners))
    {
        wintc_start_mfu_tracker_save_winners(mfu_tracker);
    }
}

static void wintc_start_mfu_tracker_load_winners(
    WinTCStartMfuTracker* mfu_tracker
)
{
    GError* error = NULL;

    // Reset all scores to 0 to start with, a clean slate
    //
    for (GList* iter = mfu_tracker->list_scores; iter; iter = iter->next)
    {
        ((WinTCStartMfuEntry*) iter->data)->score = 0;
    }

    // Grab the winners
    //
    GList* list_winners;
    gchar* winners_text = NULL;

    if (
        !wintc_profile_get_file_contents(
            WINTC_COMPONENT_SHELL,
            MFU_WINNERS_FILENAME,
            &winners_text,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    list_winners = wintc_list_read_from_string(winners_text);
    g_free(winners_text);

    // Translate to list and look up the winners, set some initial scores
    //
    gint   i;
    GList* iter;

    mfu_tracker->loading_winners = TRUE; // Will prevent saving during bumps

    for (
        iter = list_winners, i = 6;
        iter && i > 0;
        iter = iter->next, i--
    )
    {
        GList* li_entry =
            g_hash_table_lookup(
                mfu_tracker->map_cmdline_to_list_item,
                (gchar*) iter->data
            );

        if (!li_entry)
        {
            continue;
        }

        ((WinTCStartMfuEntry*) li_entry->data)->score = i * 2;

        wintc_start_mfu_tracker_bubble_entry(
            mfu_tracker,
            li_entry
        );
    }

    mfu_tracker->loading_winners = FALSE;

    g_list_free_full(list_winners, (GDestroyNotify) g_free);
}

static void wintc_start_mfu_tracker_save_winners(
    WinTCStartMfuTracker* mfu_tracker
)
{
    GError* error = NULL;

    // Get all the cmdlines for the winners
    //
    GList* list_cmd = NULL;
    GList* list_mfu = wintc_start_mfu_tracker_get_mfu_list(mfu_tracker);

    for (GList* iter = list_mfu; iter; iter = iter->next)
    {
        list_cmd =
            g_list_prepend(
                list_cmd,
                garcon_menu_item_get_command_expanded(
                    GARCON_MENU_ITEM(iter->data)
                )
            );
    }

    list_cmd = g_list_reverse(list_cmd);

    // Write out to profile
    //
    gchar* winners_file =
        wintc_list_implode_strings(list_cmd);

    if (
        !wintc_profile_set_file_contents(
            WINTC_COMPONENT_SHELL,
            MFU_WINNERS_FILENAME,
            winners_file,
            -1,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    g_free(winners_file);
    g_list_free_full(list_cmd, (GDestroyNotify) g_free);
    g_list_free(list_mfu);
}
