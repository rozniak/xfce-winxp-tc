#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/error.h"
#include "../public/fsclipbd.h"
#include "../public/fsop.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_CAN_PASTE = 1
};

enum
{
    TARGET_URI_LIST,
    TARGET_GNOME_COPIED_FILES
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_fs_clipboard_constructed(
    GObject* object
);
static void wintc_sh_fs_clipboard_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static void wintc_sh_fs_clipboard_update_state(
    WinTCShFSClipboard* fs_clipboard
);

static void cb_clipboard_contents_received(
    GtkClipboard*     clipboard,
    GtkSelectionData* selection_data,
    gpointer          user_data
);
static void cb_clipboard_targets_received(
    GtkClipboard* clipboard,
    GdkAtom*      atoms,
    gint          n_atoms,
    gpointer      user_data
);

static void cb_copymove_clear(
    GtkClipboard* clipboard,
    GObject*      owner
);
static void cb_copymove_get(
    GtkClipboard*     clipboard,
    GtkSelectionData* selection_data,
    guint             info,
    GObject*          owner
);

static void on_clipboard_owner_change(
    GtkClipboard*        self,
    GdkEventOwnerChange* event,
    gpointer             user_data
);
static void on_fs_operation_done(
    WinTCShFSOperation* self,
    gpointer            user_data
);

//
// STATIC DATA
//
static GdkAtom S_ATOM_TEXT_URI_LIST;
static GdkAtom S_ATOM_X_SPECIAL_GNOME_COPIED_FILES;

static GtkTargetEntry S_TARGETS_COPYMOVE[] = {
    {
        "text/uri-list",
        0,
        TARGET_URI_LIST
    },
    {
        "x-special/gnome-copied-files",
        0,
        TARGET_GNOME_COPIED_FILES
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCShFSClipboard
{
    GObject __parent__;

    GtkClipboard* clipboard;

    // Copymove status
    //
    GList*                 list_cm_uris;
    WinTCShFSOperationKind op_cm;

    // Paste status
    //
    GList*                 list_uris;
    WinTCShFSOperationKind operation_kind;
    GdkAtom                preferred_target;
} WinTCShFSClipboard;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShFSClipboard,
    wintc_sh_fs_clipboard,
    G_TYPE_OBJECT
)

static void wintc_sh_fs_clipboard_class_init(
    WinTCShFSClipboardClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_fs_clipboard_constructed;
    object_class->get_property = wintc_sh_fs_clipboard_get_property;

    g_object_class_install_property(
        object_class,
        PROP_CAN_PASTE,
        g_param_spec_boolean(
            "can-paste",
            "CanPaste",
            "Determines whether the clipboard has pastable content.",
            FALSE,
            G_PARAM_READABLE
        )
    );

    S_ATOM_TEXT_URI_LIST =
        gdk_atom_intern_static_string("text/uri-list");
    S_ATOM_X_SPECIAL_GNOME_COPIED_FILES =
        gdk_atom_intern_static_string("x-special/gnome-copied-files");
}

static void wintc_sh_fs_clipboard_init(
    WinTCShFSClipboard* self
)
{
    WINTC_LOG_DEBUG("shell: fs clipbd - creating clipboard");

    self->clipboard =
        gtk_clipboard_get_default(gdk_display_get_default());

    g_signal_connect(
        self->clipboard,
        "owner-change",
        G_CALLBACK(on_clipboard_owner_change),
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_fs_clipboard_constructed(
    GObject* object
)
{
    WinTCShFSClipboard* fs_clipboard = WINTC_SH_FS_CLIPBOARD(object);

    wintc_sh_fs_clipboard_update_state(fs_clipboard);

    (G_OBJECT_CLASS(wintc_sh_fs_clipboard_parent_class))
        ->constructed(object);
}

static void wintc_sh_fs_clipboard_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCShFSClipboard* fs_clipboard = WINTC_SH_FS_CLIPBOARD(object);

    switch (prop_id)
    {
        case PROP_CAN_PASTE:
            g_value_set_boolean(
                value,
                !!(fs_clipboard->list_uris)
            );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShFSClipboard* wintc_sh_fs_clipboard_new(void)
{
    // We are using a singleton instance here, but as far as programs are
    // concerned it's an ordinary object they own a reference to
    //
    // This gives some flexibility if any changes are needed in future, but
    // since the clipboard is a shared resource it makes sense to only hold
    // one instance
    //
    static WinTCShFSClipboard* singleton_clipboard = NULL;

    if (!singleton_clipboard)
    {
        singleton_clipboard =
            WINTC_SH_FS_CLIPBOARD(
                g_object_new(
                    WINTC_TYPE_SH_FS_CLIPBOARD,
                    NULL
                )
            );
    }

    g_object_ref(singleton_clipboard);

    return singleton_clipboard;
}

gboolean wintc_sh_fs_clipboard_copymove(
    WinTCShFSClipboard* fs_clipboard,
    GList*              srcs,
    gboolean            is_move,
    GError**            error
)
{
    WINTC_SAFE_REF_CLEAR(error);

    g_clear_list(
        &(fs_clipboard->list_cm_uris),
        (GDestroyNotify) g_free
    );

    // Set up our own state, and then claim the clipboard
    //
    fs_clipboard->list_cm_uris = srcs;
    fs_clipboard->op_cm        = is_move ?
                                     WINTC_SH_FS_OPERATION_MOVE :
                                     WINTC_SH_FS_OPERATION_COPY;

    return gtk_clipboard_set_with_owner(
        fs_clipboard->clipboard,
        S_TARGETS_COPYMOVE,
        G_N_ELEMENTS(S_TARGETS_COPYMOVE),
        (GtkClipboardGetFunc) cb_copymove_get,
        (GtkClipboardClearFunc) cb_copymove_clear,
        G_OBJECT(fs_clipboard)
    );
}

gboolean wintc_sh_fs_clipboard_paste(
    WinTCShFSClipboard* fs_clipboard,
    const gchar*        dest,
    GtkWindow*          wnd,
    GError**            error
)
{
    if (!(fs_clipboard->list_uris))
    {
        g_set_error(
            error,
            wintc_shell_error_quark(),
            WINTC_SHELL_ERROR_CLIPBOARD_EMPTY,
            "%s",
            "There are no files on the clipboard." // FIXME: Localise
        );

        return FALSE;
    }

    // Attempt to paste these files
    //
    WinTCShFSOperation* fs_operation =
        wintc_sh_fs_operation_new(
            fs_clipboard->list_uris,
            dest,
            fs_clipboard->operation_kind
        );

    g_signal_connect(
        fs_operation,
        "done",
        G_CALLBACK(on_fs_operation_done),
        NULL
    );

    wintc_sh_fs_operation_do(
        fs_operation,
        wnd
    );

    // If the operation is move (aka 'Cut'), we should clear the clipboard
    //
    if (fs_clipboard->operation_kind == WINTC_SH_FS_OPERATION_MOVE)
    {
        wintc_clipboard_true_clear(fs_clipboard->clipboard);
    }

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_sh_fs_clipboard_update_state(
    WinTCShFSClipboard* fs_clipboard
)
{
    WINTC_LOG_DEBUG("shell: fsclipbd - update state");

    fs_clipboard->operation_kind   = WINTC_SH_FS_OPERATION_INVALID;
    fs_clipboard->preferred_target = GDK_NONE;

    if (fs_clipboard->list_uris)
    {
        g_clear_list(
            &(fs_clipboard->list_uris),
            (GDestroyNotify) g_free
        );
    }

    g_object_notify(
        G_OBJECT(fs_clipboard),
        "can-paste"
    );

    gtk_clipboard_request_targets(
        fs_clipboard->clipboard,
        cb_clipboard_targets_received,
        fs_clipboard
    );
}

//
// CALLBACKS
//
static void cb_clipboard_contents_received(
    WINTC_UNUSED(GtkClipboard* clipboard),
    GtkSelectionData* selection_data,
    gpointer          user_data
)
{
    WinTCShFSClipboard* fs_clipboard = WINTC_SH_FS_CLIPBOARD(user_data);

    WINTC_LOG_DEBUG("shell: fsclipbd - contents received, parsing");

    // Parse the contents
    //
    const guchar* data;
    const guchar* data_end;
    gint          len         = 0;

    data     = gtk_selection_data_get_data_with_length(
                   selection_data,
                   &len
               );
    data_end = data + len;

    if (len < 0) // No data on clipboard
    {
        WINTC_LOG_DEBUG("shell: fsclipbd - apparently the contents are empty");
        return;
    }

    if (fs_clipboard->preferred_target == S_ATOM_X_SPECIAL_GNOME_COPIED_FILES)
    {
        if (g_ascii_strncasecmp((const gchar*) data, "copy\n", 5) == 0)
        {
            fs_clipboard->operation_kind = WINTC_SH_FS_OPERATION_COPY;
            data += 5;
        }
        else if (g_ascii_strncasecmp((const gchar*) data, "cut\n", 4) == 0)
        {
            fs_clipboard->operation_kind = WINTC_SH_FS_OPERATION_MOVE;
            data += 4;
        }
    }
    else
    {
        fs_clipboard->operation_kind = WINTC_SH_FS_OPERATION_COPY;
    }

    while (data < data_end)
    {
        // Search for next \n
        //
        const guchar* next_lf = memchr(data, '\n', data_end - data);

        if (!next_lf) // gnome-copied-files has no final \n
        {
            next_lf =  data_end;
        }

        // Work out how much we need to slice
        //
        gint copy_len = next_lf - data;

        if (fs_clipboard->preferred_target == S_ATOM_TEXT_URI_LIST)
        {
            copy_len--; // text/uri-list uses \r\n rather than just \n
        }

        // Slice and store the URI in our list
        //
        gchar* buf = g_malloc(copy_len + 1);

        memcpy(buf, data, copy_len);
        buf[copy_len] = 0;

        fs_clipboard->list_uris =
            g_list_append(fs_clipboard->list_uris, buf);

        // Iter
        //
        data = next_lf + 1;
    }

    g_object_notify(
        G_OBJECT(fs_clipboard),
        "can-paste"
    );
}

static void cb_clipboard_targets_received(
    GtkClipboard* clipboard,
    GdkAtom*      atoms,
    gint          n_atoms,
    gpointer      user_data
)
{
#define K_N_ATOMS_SEARCH 2

    static GdkAtom s_atoms_prioritised[K_N_ATOMS_SEARCH];

    if (s_atoms_prioritised[0] == GDK_NONE)
    {
        s_atoms_prioritised[0] = S_ATOM_X_SPECIAL_GNOME_COPIED_FILES;
        s_atoms_prioritised[1] = S_ATOM_TEXT_URI_LIST;
    }

    // Look for the atoms
    //
    WinTCShFSClipboard* fs_clipboard = WINTC_SH_FS_CLIPBOARD(user_data);

    WINTC_LOG_DEBUG("shell: fsclipbd - targets received, parsing");

    for (gint i = 0; i < K_N_ATOMS_SEARCH; i++)
    {
        GdkAtom lookfor = s_atoms_prioritised[i];

        for (gint j = 0; j < n_atoms; j++)
        {
            GdkAtom lookat = atoms[j];

            if (lookat == lookfor)
            {
                fs_clipboard->preferred_target = lookfor;
                goto doneloop;
            }
        }
    }

doneloop:
    if (fs_clipboard->preferred_target == GDK_NONE)
    {
        WINTC_LOG_DEBUG("shell: fsclipbd - no supported formats");
        return;
    }

    gtk_clipboard_request_contents(
        clipboard,
        fs_clipboard->preferred_target,
        cb_clipboard_contents_received,
        fs_clipboard
    );
}

static void cb_copymove_clear(
    WINTC_UNUSED(GtkClipboard* clipboard),
    GObject* owner
)
{
    WinTCShFSClipboard* fs_clipboard = WINTC_SH_FS_CLIPBOARD(owner);

    fs_clipboard->op_cm = WINTC_SH_FS_OPERATION_INVALID;

    g_clear_list(
        &fs_clipboard->list_cm_uris,
        (GDestroyNotify) g_free
    );
}

static void cb_copymove_get(
    WINTC_UNUSED(GtkClipboard* clipboard),
    GtkSelectionData* selection_data,
    guint             info,
    GObject*          owner
)
{
    WinTCShFSClipboard* fs_clipboard = WINTC_SH_FS_CLIPBOARD(owner);

    // Work out the size of buffer we need for the clipboard
    //
    guint buf_size = 0;

    if (info == TARGET_URI_LIST)
    {
        for (GList* iter = fs_clipboard->list_cm_uris; iter; iter = iter->next)
        {
            buf_size += 7; // file://
            buf_size += g_utf8_strlen((gchar*) iter->data, -1);
            buf_size += 2; // \r\n
        }
    }
    else // TARGET_GNOME_COPIED_FILES
    {
        if (fs_clipboard->op_cm == WINTC_SH_FS_OPERATION_COPY)
        {
            buf_size += 5; // copy\n
        }
        else // MOVE
        {
            buf_size += 4; // cut\n
        }

        for (GList* iter = fs_clipboard->list_cm_uris; iter; iter = iter->next)
        {
            buf_size += 7; // file://
            buf_size += g_utf8_strlen((gchar*) iter->data, -1);
            buf_size += 1; // \n

            // No newline for the last entry
            //
            if (!(iter->next))
            {
                buf_size--;
            }
        }
    }

    buf_size++; // Terminating null

    // Populate the buffer
    //
    gchar* buf     = g_malloc(buf_size);
    gchar* buf_ptr = buf;

    if (info == TARGET_GNOME_COPIED_FILES)
    {
        buf_ptr =
            stpcpy(
                buf_ptr,
                fs_clipboard->op_cm == WINTC_SH_FS_OPERATION_COPY ?
                    "copy\n" :
                    "cut\n"
            );
    }

    for (GList* iter = fs_clipboard->list_cm_uris; iter; iter = iter->next)
    {
        buf_ptr = stpcpy(buf_ptr, "file://");
        buf_ptr = stpcpy(buf_ptr, (gchar*) iter->data);

        if (info == TARGET_URI_LIST)
        {
            buf_ptr = stpcpy(buf_ptr, "\r\n");
        }
        else // TARGET_GNOME_COPIED_FILES
        {
            // Only add newline if this is not the last URI
            //
            if (iter->next)
            {
                buf_ptr = stpcpy(buf_ptr, "\n");
            }
        }
    }

    // Update the clipboard
    //
    gtk_selection_data_set(
        selection_data,
        info == TARGET_URI_LIST ?
            S_ATOM_TEXT_URI_LIST : S_ATOM_X_SPECIAL_GNOME_COPIED_FILES,
        8,
        (guchar*) buf,
        buf_size
    );

    g_free(buf);
}

static void on_clipboard_owner_change(
    WINTC_UNUSED(GtkClipboard* self),
    WINTC_UNUSED(GdkEventOwnerChange* event),
    gpointer user_data
)
{
    WINTC_LOG_DEBUG("shell: fsclipbd - owner changed");

    wintc_sh_fs_clipboard_update_state(
        WINTC_SH_FS_CLIPBOARD(user_data)
    );
}

static void on_fs_operation_done(
    WinTCShFSOperation* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    g_object_unref(self);
}
