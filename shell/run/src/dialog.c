#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shlang.h>

#include "application.h"
#include "dialog.h"
#include "history.h"

#define INITIAL_POS_EDGE_OFFSET 4

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCRunDialogClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCRunDialog
{
    GtkApplicationWindow __parent__;

    // UI
    //
    GtkWidget* combo;
    GtkWidget* entry;
};

//
// FORWARD DECLARATION
//
static void wintc_run_dialog_init_combobox(
    WinTCRunDialog* dialog
);

static void wintc_run_dialog_finalize(
    GObject* object
);

static void iter_run_history(
    gchar*           cmdline,
    GtkComboBoxText* combo
);

static void on_browse_button_clicked(
    GtkWidget*      button,
    WinTCRunDialog* dialog
);
static void on_cancel_button_clicked(
    GtkWidget*      button,
    WinTCRunDialog* dialog
);
static gboolean on_dialog_key_pressed(
    GtkWidget*   dialog,
    GdkEventKey* event,
    gpointer     user_data
);
static void on_dialog_map_event(
    GtkWidget*  self,
    GdkEventAny event,
    gpointer    user_data
);
static void on_ok_button_clicked(
    GtkWidget*      button,
    WinTCRunDialog* dialog
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCRunDialog,
    wintc_run_dialog,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_run_dialog_class_init(
    WinTCRunDialogClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_run_dialog_finalize;
}

static void wintc_run_dialog_init(
    WinTCRunDialog* self
)
{
    GtkWidget* box_buttons;
    GtkWidget* box_input;
    GtkWidget* box_instructions;
    GtkWidget* box_outer;
    GtkWidget* button_browse;
    GtkWidget* button_cancel;
    GtkWidget* button_ok;
    GtkWidget* combo_entry;
    GtkWidget* combo_entry_internal;
    GtkWidget* label_instruction;
    GtkWidget* label_open;
    GtkWidget* icon;

    // Set up window
    //
    gtk_widget_set_size_request(GTK_WIDGET(self), 341, 154);
    gtk_window_set_icon_name(GTK_WINDOW(self), "system-run");
    gtk_window_set_resizable(GTK_WINDOW(self), FALSE);
    gtk_window_set_title(GTK_WINDOW(self), _("Run"));
    gtk_window_set_type_hint(GTK_WINDOW(self), GDK_WINDOW_TYPE_HINT_MENU);

    g_signal_connect(
        self,
        "key-press-event",
        G_CALLBACK(on_dialog_key_pressed),
        NULL
    );

    // Create icon
    //
    icon = gtk_image_new_from_icon_name("system-run", GTK_ICON_SIZE_DND);

    // Create instruction label
    //
    label_instruction = gtk_label_new(_("Type the name of a program, folder, document, or Internet resource, and Windows will open it for you."));

    gtk_label_set_line_wrap(GTK_LABEL(label_instruction), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(label_instruction), 48);
    gtk_label_set_xalign(GTK_LABEL(label_instruction), 0.0);

    // Create "Open:" label
    //
    label_open =
        gtk_label_new(
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_ITEMIZATION)
        );

    // Create combobox entry
    //
    combo_entry          = gtk_combo_box_text_new_with_entry();
    combo_entry_internal = gtk_bin_get_child(GTK_BIN(combo_entry));

    gtk_entry_set_activates_default(
        GTK_ENTRY(combo_entry_internal),
        TRUE
    );

    // Create buttons
    // 
    button_browse = gtk_button_new_with_label(
                        wintc_lc_get_control_text(
                            WINTC_CTLTXT_BROWSE,
                            WINTC_PUNC_MOREINPUT
                        )
                    );
    button_cancel = gtk_button_new_with_label(
                        wintc_lc_get_control_text(
                            WINTC_CTLTXT_CANCEL,
                            WINTC_PUNC_NONE
                        )
                    );
    button_ok     = gtk_button_new_with_label(
                        wintc_lc_get_control_text(
                            WINTC_CTLTXT_OK,
                            WINTC_PUNC_NONE
                        )
                    );

    gtk_widget_set_can_default(button_ok, TRUE);

    g_signal_connect(
        button_browse,
        "clicked",
        G_CALLBACK(on_browse_button_clicked),
        self
    );

    g_signal_connect(
        button_cancel,
        "clicked",
        G_CALLBACK(on_cancel_button_clicked),
        self
    );

    g_signal_connect(
        button_ok,
        "clicked",
        G_CALLBACK(on_ok_button_clicked),
        self
    );

    // Create boxes
    //
    box_outer        = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    box_buttons      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    box_input        = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    box_instructions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_container_add(GTK_CONTAINER(self), box_outer);

    gtk_box_pack_start(GTK_BOX(box_instructions), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_instructions), label_instruction, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_outer), box_instructions, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(box_input), label_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_input), combo_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_outer), box_input, FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(box_buttons), button_browse, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(box_buttons), button_cancel, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(box_buttons), button_ok, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_outer), box_buttons, FALSE, FALSE, 0);

    wintc_widget_add_style_class(
        box_buttons,
        WINTC_CTL_BUTTON_BOX_CSS_CLASS
    );

    // Set OK button as default widget
    //
    gtk_window_set_default(GTK_WINDOW(self), button_ok);

    // Apply styles
    //
    // FIXME: This should not be done here, use screen CSS instead!
    //
    wintc_widget_add_css(box_buttons, "box { margin-bottom: 10px; }");
    wintc_widget_add_css(box_outer, "box { margin: 18px 11px 0px; }");
    wintc_widget_add_css(box_instructions, "box { margin-bottom: 13px; }");
    wintc_widget_add_css(box_input, "box { margin-bottom: 34px; }");

    wintc_widget_add_css(icon, "image { margin-right: 11px; }");
    wintc_widget_add_css(label_open, "label { margin-right: 11px; }");

    // Attach signal to map-event so we can position the window
    //
    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_dialog_map_event),
        NULL
    );

    // Set up instance private
    //
    self->combo = combo_entry;
    self->entry = combo_entry_internal;

    // Initialize combobox with value
    //
    wintc_run_dialog_init_combobox(self);
}

//
// FINALIZE
//
static void wintc_run_dialog_finalize(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_run_dialog_parent_class))->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_run_dialog_new(
    WinTCRunApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_RUN_DIALOG,
            "application", GTK_APPLICATION(app),
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_run_dialog_init_combobox(
    WinTCRunDialog* dialog
)
{
    GtkTreeIter   combo_iter;
    GtkTreeModel* combo_model;
    GError*       error       = NULL;
    GList*        history     = NULL;

    history = wintc_get_run_history(&error);

    if (history == NULL)
    {
        if (error != NULL)
        {
            WINTC_LOG_USER_DEBUG("Can't get run history: %s", error->message);
            g_clear_error(&error);
        }

        return;
    }

    g_list_foreach(
        history,
        (GFunc) iter_run_history,
        GTK_COMBO_BOX_TEXT(dialog->combo)
    );

    // Try to select the most recent item
    //
    combo_model = gtk_combo_box_get_model(GTK_COMBO_BOX(dialog->combo));
    
    if (
        !gtk_tree_model_get_iter_first(
            combo_model,
            &combo_iter
        )
    )
    {
        WINTC_LOG_USER_DEBUG("No recent runs.");
        return;
    }

    gtk_combo_box_set_active_iter(
        GTK_COMBO_BOX(dialog->combo),
        &combo_iter
    );
}

//
// CALLBACKS
//
static void iter_run_history(
    gchar*           cmdline,
    GtkComboBoxText* combo
)
{
    gtk_combo_box_text_append(
        combo,
        NULL,
        cmdline
    );
}

static void on_browse_button_clicked(
    WINTC_UNUSED(GtkWidget* button),
    WinTCRunDialog* dialog
)
{
    GtkWidget*     file_dialog;
    GtkFileFilter* filter_all_files;
    GtkFileFilter* filter_programs;
    gint           result;

    // Set up file filters
    //
    filter_all_files = gtk_file_filter_new();
    filter_programs  = gtk_file_filter_new();

    gtk_file_filter_set_name(filter_all_files, _("All Files"));
    gtk_file_filter_add_pattern(filter_all_files, "*");

    gtk_file_filter_set_name(filter_programs, _("Programs"));
    gtk_file_filter_add_mime_type(filter_programs, "application/x-executable");

    // Set up file dialog
    //
    file_dialog =
        gtk_file_chooser_dialog_new(
            wintc_lc_get_control_text(WINTC_CTLTXT_BROWSE, WINTC_PUNC_NONE),
            GTK_WINDOW(dialog),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
            GTK_RESPONSE_CANCEL,
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_RESPONSE_ACCEPT,
            NULL
        );

    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(file_dialog),
        "/usr/bin"
    );

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(file_dialog),
        filter_programs
    );
    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(file_dialog),
        filter_all_files
    );

    // Execute dialog and handle result
    //
    result = gtk_dialog_run(GTK_DIALOG(file_dialog));

    if (result == GTK_RESPONSE_ACCEPT)
    {
        gtk_entry_set_text(
            GTK_ENTRY(dialog->entry),
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog))
        );
    }

    gtk_widget_destroy(file_dialog);
    wintc_focus_window(GTK_WINDOW(dialog));
    gtk_widget_grab_focus(dialog->entry);
}

static void on_cancel_button_clicked(
    WINTC_UNUSED(GtkWidget* button),
    WinTCRunDialog* dialog
)
{
    gtk_window_close(GTK_WINDOW(dialog));
}

static void on_dialog_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEventAny event),
    WINTC_UNUSED(gpointer    user_data)
)
{
    // FIXME: This presumably only works on X11, Wayland could probably use
    //        some layer shell hack to anchor the window?
    //
    GdkDisplay*  display = gdk_display_get_default();
    GdkMonitor*  monitor = gdk_display_get_primary_monitor(display);
    GdkRectangle monitor_rect;
    GtkWindow*   window  = GTK_WINDOW(self);
    GdkRectangle window_rect;

    gdk_monitor_get_workarea(
        monitor,
        &monitor_rect
    );
    gdk_window_get_frame_extents(
        gtk_widget_get_window(self),
        &window_rect
    );

    gtk_window_move(
        window,
        monitor_rect.x + INITIAL_POS_EDGE_OFFSET,
        monitor_rect.y + monitor_rect.height - window_rect.height - INITIAL_POS_EDGE_OFFSET
    );
}

static gboolean on_dialog_key_pressed(
    GtkWidget*   dialog,
    GdkEventKey* event,
    WINTC_UNUSED(gpointer user_data)
)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        gtk_window_close(GTK_WINDOW(dialog));
        return TRUE;
    }

    return FALSE;
}

static void on_ok_button_clicked(
    WINTC_UNUSED(GtkWidget* button),
    WinTCRunDialog* dialog
)
{
    GtkEntry*    entry           = GTK_ENTRY(dialog->entry);
    GError*      error           = NULL;
    const gchar* cmdline         = gtk_entry_get_text(entry);

    if (wintc_launch_command(cmdline, &error))
    {
        if (!wintc_append_run_history(cmdline, &error))
        {
            WINTC_LOG_USER_DEBUG("Can't update history: %s", error->message);
            g_clear_error(&error);
        }

        gtk_window_close(GTK_WINDOW(dialog));

        return;
    }

    // Command launch failed, what was the error?!
    //
    gchar* message;

    WINTC_LOG_USER_DEBUG("Run command failed: %s", error->message);

    switch (error->code)
    {
        case G_FILE_ERROR_NOENT:
            message =
                g_strdup_printf(
                    _("Windows cannot find '%s'. Make sure you typed the name correctly, and then try again. To search for a file, click the Start button, and then click Search."),
                    cmdline
                );
            break;

        case G_FILE_ERROR_NOSYS:
            message =
                g_strdup_printf(
                    _("%s\n\nThe parameter is incorrect"),
                    cmdline
                );
            break;

        default:
            message =
                g_strdup_printf(
                    _("Execution of '%s' failed."),
                    cmdline
                );
            break;
    }

    wintc_messagebox_show(
        GTK_WINDOW(dialog),
        message,
        cmdline,
        GTK_BUTTONS_OK,
        GTK_MESSAGE_ERROR
    );

    g_clear_error(&error);
    g_free(message);

    // Re-focus the window
    //
    wintc_focus_window(GTK_WINDOW(dialog));
}
