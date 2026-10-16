#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCmdWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCCmdWindow
{
    GtkApplicationWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCCmdWindow,
    wintc_cmd_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_cmd_window_class_init(
    WINTC_UNUSED(WinTCCmdWindowClass* klass)
) {}

static void wintc_cmd_window_init(
    WinTCCmdWindow* self
)
{
    GtkWidget* term = vte_terminal_new();

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        658,
        300
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        term
    );

    // Try to set the font to the OEM 8x16 one
    //
    PangoFontDescription* desc =
        pango_font_description_from_string("PxPlus IBM VGA 8x16 12");

    vte_terminal_set_font(VTE_TERMINAL(term), desc);

    pango_font_description_free(desc);

    // Launch the user's shell
    //
    static gchar* s_cmd[] = {
        "/usr/bin/env",
        NULL, // $SHELL
        NULL
    };

    s_cmd[1] = g_strdup(g_getenv("SHELL"));

    vte_terminal_spawn_async(
        VTE_TERMINAL(term),
        VTE_PTY_DEFAULT,
        g_get_home_dir(),
        s_cmd,
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        NULL,
        NULL
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cmd_window_new(
    WinTCCmdApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_HELLO_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Command Prompt", // FIXME: Localise
            NULL
        )
    );
}
