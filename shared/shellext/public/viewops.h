/** @file */

#ifndef __SHELLEXT_VIEWOPS_H__
#define __SHELLEXT_VIEWOPS_H__

#include <glib.h>

//
// FORWARD DECLARATIONS
//
typedef struct _WinTCIShextView WinTCIShextView;
typedef struct _WinTCShextOperation WinTCShextOperation;

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SHEXT_OP_PRIORITY_NONE,
    WINTC_SHEXT_OP_PRIORITY_PRIMARY,
    WINTC_SHEXT_OP_PRIORITY_SECONDARY
} WinTCShextOperationPriority;

typedef enum
{
    WINTC_SHEXT_OP_INVALID = 0,

    WINTC_SHEXT_KNOWN_OP_OPEN = 1,
    WINTC_SHEXT_KNOWN_OP_OPEN_WITH,
    WINTC_SHEXT_KNOWN_OP_RUN_AS,
    WINTC_SHEXT_KNOWN_OP_CUT,
    WINTC_SHEXT_KNOWN_OP_COPY,
    WINTC_SHEXT_KNOWN_OP_PASTE,
    WINTC_SHEXT_KNOWN_OP_PASTE_SHORTCUT,
    WINTC_SHEXT_KNOWN_OP_CREATE_SHORTCUT,
    WINTC_SHEXT_KNOWN_OP_DELETE,
    WINTC_SHEXT_KNOWN_OP_RENAME,
    WINTC_SHEXT_KNOWN_OP_SEND_TO,
    WINTC_SHEXT_KNOWN_OP_MOVE_TO,
    WINTC_SHEXT_KNOWN_OP_COPY_TO,
    WINTC_SHEXT_KNOWN_OP_PROPERTIES,

    WINTC_SHEXT_OP_NEW = 80, // 80-99 for 'New...' menu items, max 20 items

    WINTC_SHEXT_OP_CUSTOM = 100
} WinTCShextOperationId;

#define WINTC_SHEXT_OP_IS_NEW_OP(op) \
    (op >= WINTC_SHEXT_OP_NEW && op < WINTC_SHEXT_OP_CUSTOM)

//
// PUBLIC STRUCTURES
//
typedef gboolean (*WinTCShextOperationFunc) (
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    GError**             error
);

struct _WinTCShextOperation
{
    WinTCShextOperationFunc func;
    gpointer                priv;
};

#endif
