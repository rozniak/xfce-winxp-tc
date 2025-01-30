#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>
#include <wintc/shlang.h>

#include "application.h"
#include "window.h"

#define DOCUMENT_NAME (wnd->file_uri ? wnd->file_uri : _("Untitled"))

//
// PRIVATE ENUMS
//
enum
{
    PROP_FILE_URI = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_notepad_window_finalize(
    GObject* object
);
static void wintc_notepad_window_get_property(
    GObject*      object,
    guint         prop_id,
    GValue*       value,
    GParamSpec*   pspec
);
static void wintc_notepad_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_notepad_window_close_document(
    WinTCNotepadWindow* wnd
);
static void wintc_notepad_window_update_title(
    WinTCNotepadWindow* wnd
);

static GtkWidget* create_file_chooser_dialog(
    WinTCNotepadWindow*  wnd,
    GtkFileChooserAction action
);

static void action_notimpl(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void action_about(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_exit(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_new(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_open(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_save(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_save_as(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static gboolean on_window_delete_event(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static gboolean on_window_map_event(
    GtkWidget*   self,
    GdkEventAny* event,
    gpointer     user_data
);

//
// STATIC DATA
//
static GActionEntry s_window_actions[] = {
    {
        .name           = "notimpl",
        .activate       = action_notimpl,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },

    {
        .name           = "about",
        .activate       = action_about,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "exit",
        .activate       = action_exit,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "new",
        .activate       = action_new,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "open",
        .activate       = action_open,
        .parameter_type = "s",
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "save",
        .activate       = action_save,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "save-as",
        .activate       = action_save_as,
        .parameter_type = "s",
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotepadWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCNotepadWindow
{
    GtkApplicationWindow __parent__;

    GtkTextBuffer* text_buffer;
    GtkWidget*     text_view;

    gchar* file_uri;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCNotepadWindow,
    wintc_notepad_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_notepad_window_class_init(
    WinTCNotepadWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_notepad_window_finalize;
    object_class->get_property = wintc_notepad_window_get_property;
    object_class->set_property = wintc_notepad_window_set_property;

    g_object_class_install_property(
        object_class,
        PROP_FILE_URI,
        g_param_spec_string(
            "file-uri",
            "FileUri",
            "The URI of the currently open file, or file to be opened.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_notepad_window_init(
    WinTCNotepadWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box;

    // Define GActions
    //
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        s_window_actions,
        G_N_ELEMENTS(s_window_actions),
        self
    );

    // FIXME: Default to this size, and should remember the last size the
    //        window was
    //
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        400,
        240
    );
    gtk_window_set_title(GTK_WINDOW(self), _("Notepad"));

    // Initialize UI
    //
    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/notepad/notepad.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main-box"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    // Pull out text view stuff
    //
    self->text_view   = GTK_WIDGET(
                            gtk_builder_get_object(builder, "text-edit")
                        );
    self->text_buffer = gtk_text_view_get_buffer(
                            GTK_TEXT_VIEW(self->text_view)
                        );

    g_object_unref(G_OBJECT(builder));

    // Bind signals
    //
    g_signal_connect(
        self,
        "delete-event",
        G_CALLBACK(on_window_delete_event),
        NULL
    );
    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notepad_window_finalize(
    GObject* object
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(object);

    g_free(wnd->file_uri);

    (G_OBJECT_CLASS(wintc_notepad_window_parent_class))->finalize(object);
}

static void wintc_notepad_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(object);

    switch (prop_id)
    {
        case PROP_FILE_URI:
            g_value_set_string(value, wnd->file_uri);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_notepad_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(object);

    switch (prop_id)
    {
        case PROP_FILE_URI:
            wnd->file_uri = g_strdup(g_value_get_string(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notepad_window_new(
    WinTCNotepadApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NOTEPAD_WINDOW,
            "application", GTK_APPLICATION(app),
            NULL
        )
    );
}

GtkWidget* wintc_notepad_window_new_with_uri(
    WinTCNotepadApplication* app,
    const gchar*             uri
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NOTEPAD_WINDOW,
            "application", GTK_APPLICATION(app),
            "file-uri",    uri,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_notepad_window_close_document(
    WinTCNotepadWindow* wnd
)
{
    if (!gtk_text_buffer_get_modified(wnd->text_buffer))
    {
        return TRUE;
    }

    // Prompt user
    //
    GAction*   action;
    GtkWidget* dlg;
    gint       response;

    dlg =
        gtk_message_dialog_new(
            GTK_WINDOW(wnd),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            _("The text in the %s file has changed.\n\nDo you want to save the changes?"),
            DOCUMENT_NAME
        );

    gtk_dialog_add_buttons(
        GTK_DIALOG(dlg),
        wintc_lc_get_control_text(WINTC_CTLTXT_YES, WINTC_PUNC_NONE),
        GTK_RESPONSE_YES,
        wintc_lc_get_control_text(WINTC_CTLTXT_NO, WINTC_PUNC_NONE),
        GTK_RESPONSE_NO,
        wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
        GTK_RESPONSE_CANCEL,
        NULL
    );
    gtk_window_set_title(GTK_WINDOW(dlg), _("Notepad"));

    response = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);

    switch (response)
    {
        case GTK_RESPONSE_YES:
            action = 
                g_action_map_lookup_action(G_ACTION_MAP(wnd), "save");

            g_action_activate(action, NULL);

            // Check if we actually saved
            //
            if (gtk_text_buffer_get_modified(wnd->text_buffer))
            {
                return FALSE; // Not saved! Cancel.
            }

            return TRUE;

        case GTK_RESPONSE_NO:
            return TRUE;

        case GTK_RESPONSE_CANCEL:
            return FALSE;
    }

    return FALSE;
}

static void wintc_notepad_window_update_title(
    WinTCNotepadWindow* wnd
)
{
    // FIXME: Double check this works with localization
    //
    gchar* title =
        g_strdup_printf(
            "%s - %s",
            DOCUMENT_NAME,
            _("Notepad")
        );

    gtk_window_set_title(
        GTK_WINDOW(wnd),
        title
    );

    g_free(title);
}

static GtkWidget* create_file_chooser_dialog(
    WinTCNotepadWindow*  wnd,
    GtkFileChooserAction action
)
{
    GtkWidget*     dlg;
    GtkFileFilter* filter_all_files;
    GtkFileFilter* filter_txt;

    if (
        action != GTK_FILE_CHOOSER_ACTION_OPEN &&
        action != GTK_FILE_CHOOSER_ACTION_SAVE
    )
    {
        g_critical("Invalid file chooser action: %d", action);
        return NULL;
    }

    // Set up file filters
    // FIXME: All Files should probs be in comgtk, it's used in run as
    //        well
    // 
    filter_all_files = gtk_file_filter_new();
    filter_txt       = gtk_file_filter_new();

    gtk_file_filter_set_name(filter_all_files, _("All Files"));
    gtk_file_filter_add_pattern(filter_all_files, "*");

    gtk_file_filter_set_name(filter_txt, _("Text Documents (*.txt)"));
    gtk_file_filter_add_pattern(filter_txt, "*.txt");

    // Set up file dialog
    //
    WinTCControlTextId btn   = action == GTK_FILE_CHOOSER_ACTION_OPEN ?
                                   WINTC_CTLTXT_OPEN : WINTC_CTLTXT_SAVE;
    WinTCControlTextId title = action == GTK_FILE_CHOOSER_ACTION_OPEN ?
                                   WINTC_CTLTXT_OPEN : WINTC_CTLTXT_SAVEAS;

    dlg =
        gtk_file_chooser_dialog_new(
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_WINDOW(wnd),
            action,
            wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
            GTK_RESPONSE_CANCEL,
            wintc_lc_get_control_text(btn, WINTC_PUNC_NONE),
            GTK_RESPONSE_ACCEPT,
            NULL
        );

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dlg),
        filter_txt
    );
    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dlg),
        filter_all_files
    );

    gtk_window_set_title(
        GTK_WINDOW(dlg),
        wintc_lc_get_control_text(title, WINTC_PUNC_NONE)
    );

    return dlg;
}

//
// CALLBACKS
//
static void action_notimpl(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    WINTC_UNUSED(gpointer       user_data)
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    GError* error = NULL;

    g_set_error(
        &error,
        WINTC_GENERAL_ERROR,
        WINTC_GENERAL_ERROR_NOTIMPL,
        "%s",
        "Action not implemented."
    );

    wintc_nice_error_and_clear(&error, GTK_WINDOW(wnd));
}

static void action_about(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    wintc_sh_about(GTK_WINDOW(wnd), "Notepad", NULL, "wintc-notepad");
}

static void action_exit(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    if (wintc_notepad_window_close_document(wnd))
    {
        gtk_widget_destroy(GTK_WIDGET(wnd));
    }
}

static void action_new(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    if (!wintc_notepad_window_close_document(wnd))
    {
        return;
    }

    // Blank out buffer
    //
    gtk_text_buffer_set_text(
        wnd->text_buffer,
        "",
        -1
    );
    gtk_text_buffer_set_modified(
        wnd->text_buffer,
        FALSE
    );

    wnd->file_uri = NULL;
    wintc_notepad_window_update_title(wnd);
}

static void action_open(
    WINTC_UNUSED(GSimpleAction* action),
    GVariant* parameter,
    gpointer  user_data
)
{
    gchar*              uri = NULL;
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    if (!wintc_notepad_window_close_document(wnd))
    {
        return;
    }

    // FIXME: I would use a 'maybe type' here, but it seems like a GTK bug that
    //        the stock GtkActionables (eg. GtkMenuItem) thinks that as long as
    //        there is a parameter type defined, NULL isn't acceptable! So for
    //        the time being we use an empty string
    //
    if (g_strcmp0(g_variant_get_string(parameter, NULL), "") == 0)
    {
        GtkWidget* dlg =
            create_file_chooser_dialog(wnd, GTK_FILE_CHOOSER_ACTION_OPEN);

        // Execute dialog
        //
        if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
        {
            uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dlg));
        }

        gtk_widget_destroy(dlg);
    }
    else
    {
        uri = g_strdup(g_variant_get_string(parameter, NULL));
    }

    if (!uri)
    {
        return;
    }

    // FIXME: All this logic here is preliminary:
    //          - Only supporting file://
    //          - Just doing this in the current thread
    //
    //        Will make sense to have a lib function for reading file contents
    //        from a given URI, and supporting threading
    //
    const char* uri_scheme = g_uri_peek_scheme(uri);

    if (g_strcmp0(uri_scheme, "file") == 0)
    {
        GError* error         = NULL;
        gchar*  file_contents = NULL;
        gchar*  file_path;

        if ((file_path = g_filename_from_uri(uri, NULL, &error)) != NULL)
        {
            if (g_file_get_contents(file_path, &file_contents, NULL, &error))
            {
                // FIXME: GtkTextBuffer only accepts UTF-8 valid content, which
                //        means we can't just open any old file like notepad
                //        normally can atm, will have to think of a workaround
                //        or alternative later
                //
                if (g_utf8_validate(file_contents, -1, NULL))
                {
                    gtk_text_buffer_set_text(
                        wnd->text_buffer,
                        file_contents,
                        -1
                    );
                    gtk_text_buffer_set_modified(
                        wnd->text_buffer,
                        FALSE
                    );

                    g_free(wnd->file_uri);
                    wnd->file_uri = g_strdup(uri);

                    wintc_notepad_window_update_title(wnd);
                }
                else
                {
                    wintc_messagebox_show(
                        GTK_WINDOW(wnd),
                        "Sorry, only UTF-8 valid files are supported!",
                        "Not Implemented",
                        GTK_BUTTONS_OK,
                        GTK_MESSAGE_ERROR
                    );
                }
            }
            else
            {
                // Check if this was a file not found error, and if so give the
                // user the chance to create a new file
                //
                if (error->code == G_FILE_ERROR_NOENT)
                {
                    GtkWidget* dlg;
                    gint       response;

                    dlg =
                        gtk_message_dialog_new(
                            GTK_WINDOW(wnd),
                            GTK_DIALOG_MODAL,
                            GTK_MESSAGE_WARNING,
                            GTK_BUTTONS_NONE,
                            _("Cannot find the %s file.\n\nDo you want to create a new file?"),
                            uri
                        );

                    gtk_dialog_add_buttons(
                        GTK_DIALOG(dlg),
                        wintc_lc_get_control_text(
                            WINTC_CTLTXT_YES,
                            WINTC_PUNC_NONE
                        ),
                        GTK_RESPONSE_YES,
                        wintc_lc_get_control_text(
                            WINTC_CTLTXT_NO,
                            WINTC_PUNC_NONE
                        ),
                        GTK_RESPONSE_NO,
                        wintc_lc_get_control_text(
                            WINTC_CTLTXT_CANCEL,
                            WINTC_PUNC_NONE
                        ),
                        GTK_RESPONSE_CANCEL,
                        NULL
                    );

                    gtk_window_set_title(GTK_WINDOW(dlg), _("Notepad"));

                    response = gtk_dialog_run(GTK_DIALOG(dlg));
                    gtk_widget_destroy(dlg);

                    switch (response)
                    {
                        case GTK_RESPONSE_YES:
                            g_clear_error(&error);

                            if (
                                !g_file_set_contents(
                                    file_path,
                                    "",
                                    -1,
                                    &error
                                )
                            )
                            {
                                wintc_display_error_and_clear(
                                    &error,
                                    GTK_WINDOW(wnd)
                                );
                            }
                            break;

                        case GTK_RESPONSE_NO:
                            g_action_activate(
                                g_action_map_lookup_action(
                                    G_ACTION_MAP(wnd),
                                    "new"
                                ),
                                NULL
                            );
                            break;

                        // Clean up and close immediately!
                        //
                        case GTK_RESPONSE_CANCEL:
                            g_free(uri);
                            g_free(file_contents);
                            g_free(file_path);
                            gtk_widget_destroy(GTK_WIDGET(wnd));
                            return;
                    }
                }
                else
                {
                    wintc_display_error_and_clear(
                        &error,
                        GTK_WINDOW(wnd)
                    );
                }
            }
        }
        else
        {
            wintc_display_error_and_clear(
                &error,
                GTK_WINDOW(wnd)
            );
        }

        g_free(file_contents);
        g_free(file_path);
    }
    else
    {
        wintc_messagebox_show(
            GTK_WINDOW(wnd),
            "Sorry, only local files are supported at the moment!",
            "Not Implemented",
            GTK_BUTTONS_OK,
            GTK_MESSAGE_ERROR
        );
    }

    g_free(uri);
}

static void action_save(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer  user_data
)
{
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    // Thin wrapper into save as, with or without a filename
    //
    GAction* action_save_as =
        g_action_map_lookup_action(G_ACTION_MAP(wnd), "save-as");

    if (wnd->file_uri)
    {
        g_action_activate(action_save_as, g_variant_new_string(wnd->file_uri));
    }
    else
    {
        g_action_activate(action_save_as, g_variant_new_string(""));
    }
}

static void action_save_as(
    WINTC_UNUSED(GSimpleAction* action),
    GVariant* parameter,
    gpointer  user_data
)
{
    gchar*              uri = NULL;
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(user_data);

    // If there's no filename, then we must spawn the dialog
    //
    if (g_strcmp0(g_variant_get_string(parameter, NULL), "") == 0)
    {
        GtkWidget* dlg =
            create_file_chooser_dialog(wnd, GTK_FILE_CHOOSER_ACTION_SAVE);

        // Execute dialog
        //
        if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
        {
            uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dlg));
        }

        gtk_widget_destroy(dlg);
    }
    else
    {
        uri = g_strdup(g_variant_get_string(parameter, NULL));
    }

    if (!uri)
    {
        return;
    }

    // FIXME: Refer to action_open about the file:// limitation stuff
    //
    const gchar* uri_scheme = g_uri_peek_scheme(uri);

    if (g_strcmp0(uri_scheme, "file") == 0)
    {
        GError*     error         = NULL;
        gchar*      file_contents = NULL;
        gchar*      file_path;
        GtkTextIter iter_end;
        GtkTextIter iter_start;

        gtk_text_buffer_get_end_iter(wnd->text_buffer, &iter_end);
        gtk_text_buffer_get_start_iter(wnd->text_buffer, &iter_start);

        file_contents =
            gtk_text_buffer_get_text(
                wnd->text_buffer,
                &iter_start,
                &iter_end,
                FALSE
            );

        if ((file_path = g_filename_from_uri(uri, NULL, &error)) != NULL)
        {
            if (g_file_set_contents(file_path, file_contents, -1, &error))
            {
                g_free(wnd->file_uri);
                wnd->file_uri = g_strdup(uri);

                gtk_text_buffer_set_modified(
                    wnd->text_buffer,
                    FALSE
                );

                wintc_notepad_window_update_title(wnd);
            }
            else
            {
                wintc_display_error_and_clear(&error, GTK_WINDOW(wnd));
            }
        }
        else
        {
            wintc_display_error_and_clear(&error, GTK_WINDOW(wnd));
        }

        g_free(file_contents);
        g_free(file_path);
    }
    else
    {
        wintc_messagebox_show(
            GTK_WINDOW(wnd),
            "Sorry, only local files are supported at the moment!",
            "Not Implemented",
            GTK_BUTTONS_OK,
            GTK_MESSAGE_ERROR
        );
    }

    g_free(uri);
}

static gboolean on_window_delete_event(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent* event),
    WINTC_UNUSED(gpointer  user_data)
)
{
    GAction* action =
        g_action_map_lookup_action(G_ACTION_MAP(widget), "exit");

    g_action_activate(action, NULL);

    return TRUE;
}

static gboolean on_window_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEventAny* event),
    WINTC_UNUSED(gpointer     user_data)
)
{
    GAction*            action;
    WinTCNotepadWindow* wnd = WINTC_NOTEPAD_WINDOW(self);

    wintc_notepad_window_update_title(wnd);

    // Handle opening file here
    //
    if (wnd->file_uri == NULL)
    {
        return TRUE;
    }

    action = g_action_map_lookup_action(G_ACTION_MAP(wnd), "open");

    g_action_activate(
        action,
        g_variant_new_string(wnd->file_uri)
    );

    return TRUE;
}
