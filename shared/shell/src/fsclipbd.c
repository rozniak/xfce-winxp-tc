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

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCShFSClipboard
{
    GObject __parent__;

    GtkClipboard*          clipboard;
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
