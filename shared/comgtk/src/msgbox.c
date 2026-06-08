#include <glib.h>
#include <gtk/gtk.h>

#include "../public/builder.h"
#include "../public/msgbox.h"
#include "../public/styles.h"
#include "../public/widget.h"
#include "../public/window.h"

//
// FORWARD DECLARATIONS
//
static GtkWidget* wintc_messagebox_button_new(
    const gchar* text,
    gint         response
);
static void wintc_messagebox_init_styles(void);

static void on_messagebox_button_clicked(
    GtkButton* self,
    gpointer   user_data
);

//
// PUBLIC FUNCTIONS
//
gint wintc_messagebox_show(
    GtkWindow*     parent,
    const gchar*   text,
    const gchar*   caption,
    GtkButtonsType buttons,
    GtkMessageType type
)
{
    GtkWidget* dlg =
        gtk_dialog_new_with_buttons(
            caption,
            parent,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            NULL,
            NULL
        );

    GtkWidget* dlg_content =
        gtk_dialog_get_content_area(GTK_DIALOG(dlg));

    wintc_messagebox_init_styles();

    gtk_window_set_resizable(GTK_WINDOW(dlg), FALSE);
    wintc_widget_add_style_class(dlg, "wintc-msgbox");

    // Decide the icon
    //
    const gchar* icon_name = NULL;

    switch (type)
    {
        case GTK_MESSAGE_INFO:     icon_name = "dialog-information"; break;
        case GTK_MESSAGE_WARNING:  icon_name = "dialog-warning";     break;
        case GTK_MESSAGE_QUESTION: icon_name = "dialog-question";    break;
        case GTK_MESSAGE_ERROR:    icon_name = "dialog-error";       break;
        default: break;
    }

    // Insert the content area
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource("/uk/oddmatics/wintc/comgtk/msgbox.ui");

    GtkWidget* box_main   = NULL;
    GtkWidget* img_icon   = NULL;
    GtkWidget* label_text = NULL;

    wintc_builder_get_objects(
        builder,
        "box-main",   &box_main,
        "img-icon",   &img_icon,
        "label-text", &label_text,
        NULL
    );

    if (icon_name)
    {
        gtk_image_set_from_icon_name(
            GTK_IMAGE(img_icon),
            icon_name,
            GTK_ICON_SIZE_DIALOG
        );
    }

    gtk_label_set_text(
        GTK_LABEL(label_text),
        text
    );

    gtk_container_add(
        GTK_CONTAINER(dlg_content),
        box_main
    );

    g_object_unref(builder);

    // Lock the label height
    //
    wintc_widget_set_size_request_natural(label_text);

    // Insert the buttons
    //
    // FIXME: We probably have to bundle localisations for these buttons here
    //        because we cannot link to shlang
    //
    GtkWidget* box_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_halign(box_buttons, GTK_ALIGN_CENTER);

    switch (buttons)
    {
        case GTK_BUTTONS_NONE:
        case GTK_BUTTONS_OK:
        case GTK_BUTTONS_CLOSE:
        case GTK_BUTTONS_CANCEL:
            gtk_container_add(
                GTK_CONTAINER(box_buttons),
                wintc_messagebox_button_new(
                    "OK",
                    GTK_RESPONSE_OK
                )
            );
            break;

        case GTK_BUTTONS_YES_NO:
            gtk_container_add(
                GTK_CONTAINER(box_buttons),
                wintc_messagebox_button_new(
                    "Yes",
                    GTK_RESPONSE_YES
                )
            );
            gtk_container_add(
                GTK_CONTAINER(box_buttons),
                wintc_messagebox_button_new(
                    "No",
                    GTK_RESPONSE_NO
                )
            );
            break;

        case GTK_BUTTONS_OK_CANCEL:
            gtk_container_add(
                GTK_CONTAINER(box_buttons),
                wintc_messagebox_button_new(
                    "OK",
                    GTK_RESPONSE_OK
                )
            );
            gtk_container_add(
                GTK_CONTAINER(box_buttons),
                wintc_messagebox_button_new(
                    "Cancel",
                    GTK_RESPONSE_CANCEL
                )
            );
            break;
    }

    gtk_container_add(
        GTK_CONTAINER(dlg_content),
        box_buttons
    );

    // Show!
    //
    gtk_widget_show_all(dlg_content);

    gint response = gtk_dialog_run(GTK_DIALOG(dlg));

    gtk_widget_destroy(dlg);

    return response;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_messagebox_init_styles(void)
{
    static gboolean s_already_done = FALSE;

    if (s_already_done)
    {
        return;
    }

    GtkCssProvider* css_msgbox = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_msgbox,
        "/uk/oddmatics/wintc/comgtk/msgbox.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_msgbox),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    s_already_done = TRUE;
}

static GtkWidget* wintc_messagebox_button_new(
    const gchar* text,
    gint         response
)
{
    GtkWidget* button = gtk_button_new_with_label(text);

    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(on_messagebox_button_clicked),
        GINT_TO_POINTER(response)
    );

    return button;
}

//
// CALLBACKS
//
static void on_messagebox_button_clicked(
    GtkButton* self,
    gpointer   user_data
)
{
    GtkWindow* wnd = wintc_widget_get_toplevel_window(GTK_WIDGET(self));

    gtk_dialog_response(
        GTK_DIALOG(wnd),
        GPOINTER_TO_INT(user_data)
    );
}
