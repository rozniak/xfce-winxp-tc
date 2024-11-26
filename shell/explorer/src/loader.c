#include <glib.h>
#include <wintc/comgtk.h>

#include "loader.h"
#include "sidebars/favside.h"
#include "sidebars/fldrside.h"
#include "sidebars/srchside.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExplorerLoaderClass
{
    GObjectClass __parent__;
};

struct _WinTCExplorerLoader
{
    GObject __parent__;

    // State
    //
    GHashTable* map_sidebar_id_to_type;
};

//
// FORWARD DECLARATIONS
//
static void wintc_explorer_loader_dispose(
    GObject* object
);

static void map_sidebar_type(
    WinTCExplorerLoader* loader,
    const gchar*         sidebar_id,
    GType                type
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCExplorerLoader,
    wintc_explorer_loader,
    G_TYPE_OBJECT
)

static void wintc_explorer_loader_class_init(
    WinTCExplorerLoaderClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_explorer_loader_dispose;
}

static void wintc_explorer_loader_init(
    WINTC_UNUSED(WinTCExplorerLoader* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_explorer_loader_dispose(
    GObject* object
)
{
    WinTCExplorerLoader* loader = WINTC_EXPLORER_LOADER(object);

    g_hash_table_unref(g_steal_pointer(&(loader->map_sidebar_id_to_type)));

    (G_OBJECT_CLASS(wintc_explorer_loader_parent_class))->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerLoader* wintc_explorer_loader_new(void)
{
    return WINTC_EXPLORER_LOADER(
        g_object_new(WINTC_TYPE_EXPLORER_LOADER, NULL)
    );
}

void wintc_explorer_loader_load_extensions(
    WinTCExplorerLoader* loader
)
{
    if (loader->map_sidebar_id_to_type)
    {
        g_critical("explorer: attempt to load extensions twice");
        return;
    }

    loader->map_sidebar_id_to_type =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            g_free
        );

    // Insert the built-in sidebars
    //
    map_sidebar_type(
        loader,
        WINTC_EXPLORER_SIDEBAR_ID_FAVORITES,
        WINTC_TYPE_EXP_FAVORITES_SIDEBAR
    );
    map_sidebar_type(
        loader,
        WINTC_EXPLORER_SIDEBAR_ID_FOLDERS,
        WINTC_TYPE_EXP_FOLDERS_SIDEBAR
    );
    map_sidebar_type(
        loader,
        WINTC_EXPLORER_SIDEBAR_ID_SEARCH,
        WINTC_TYPE_EXP_SEARCH_SIDEBAR
    );

    //
    // TODO: Load sidebars from external libs
    //
}

GType wintc_explorer_loader_lookup_sidebar_type(
    WinTCExplorerLoader* loader,
    const gchar*         sidebar_id
)
{
    if (!sidebar_id)
    {
        return 0;
    }

    GType* sidebar_type =
        g_hash_table_lookup(
            loader->map_sidebar_id_to_type,
            sidebar_id
        );

    if (!sidebar_type)
    {
        g_critical("explorer: no sidebar found for %s", sidebar_id);
        return 0;
    }

    return *sidebar_type;
}

//
// PRIVATE FUNCTIONS
//
static void map_sidebar_type(
    WinTCExplorerLoader* loader,
    const gchar*         sidebar_id,
    GType                type
)
{
    // This might be crap, but GHashTable operates with pointers - the
    // X_TO_POINTER macros aren't gonna help with GType because it's too large
    // so actually allocating them here!!
    //
    // I say 'might' be crap, in all honesty it IS crap but I'm not sure I
    // really give a monkeys anyway
    //
    GType* alloc_type = g_malloc(sizeof(GType));

    memcpy(alloc_type, &type, sizeof(sizeof(GType)));

    g_hash_table_insert(
        loader->map_sidebar_id_to_type,
        g_strdup(sidebar_id),
        alloc_type
    );
}
