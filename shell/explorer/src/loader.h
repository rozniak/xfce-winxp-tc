#ifndef __LOADER_H__
#define __LOADER_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExplorerLoaderClass WinTCExplorerLoaderClass;
typedef struct _WinTCExplorerLoader      WinTCExplorerLoader;

#define WINTC_TYPE_EXPLORER_LOADER            (wintc_explorer_loader_get_type())
#define WINTC_EXPLORER_LOADER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXPLORER_LOADER, WinTCExplorerLoader))
#define WINTC_EXPLORER_LOADER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXPLORER_LOADER, WinTCExplorerLoaderClass))
#define IS_WINTC_EXPLORER_LOADER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXPLORER_LOADER))
#define IS_WINTC_EXPLORER_LOADER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXPLORER_LOADER))
#define WINTC_EXPLORER_LOADER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXPLORER_LOADER, WinTCExploerLoader))

GType wintc_explorer_loader_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerLoader* wintc_explorer_loader_new(void);

void wintc_explorer_loader_load_extensions(
    WinTCExplorerLoader* loader
);
GType wintc_explorer_loader_lookup_sidebar_type(
    WinTCExplorerLoader* loader,
    const gchar*         sidebar_id
);

#endif
