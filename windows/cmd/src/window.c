#include <gio/gunixinputstream.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_cmd_window_dispose(
    GObject* object
);
static gboolean cb_io_watch_pty(
    GIOChannel*  source,
    GIOCondition condition,
    gpointer     user_data
);
static void cb_spawn_pty(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void cb_watch_pid_shell(
    GPid     pid,
    gint     wait_status,
    gpointer user_data
);

static void on_window_map_event(
    GtkWidget* self,
    GdkEvent*  event,
    gpointer   user_data
);

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

    // State
    //
    GPid    pid_shell;
    VtePty* pty;
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
    WinTCCmdWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_cmd_window_dispose;
}

static void wintc_cmd_window_init(
    WinTCCmdWindow* self
)
{
    GError*    error     = NULL;
    GtkWidget* scrollwnd = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* term      = vte_terminal_new();

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        658,
        300
    );

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scrollwnd),
        GTK_POLICY_NEVER,
        GTK_POLICY_ALWAYS
    );
    gtk_scrolled_window_set_overlay_scrolling(
        GTK_SCROLLED_WINDOW(scrollwnd),
        FALSE
    );

    gtk_container_add(
        GTK_CONTAINER(scrollwnd),
        term
    );
    gtk_container_add(
        GTK_CONTAINER(self),
        scrollwnd
    );

    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );

    // Create the PTY...
    //
    self->pty = vte_pty_new_sync(VTE_PTY_DEFAULT, NULL, &error);

    if (!(self->pty))
    {
        wintc_display_error_and_clear(&error, NULL);
        return;
    }

    // ...attach to it to do filtering on incoming data
    //
    GIOChannel* channel =
        g_io_channel_unix_new(vte_pty_get_fd(self->pty));

    g_io_channel_set_encoding(channel, NULL, NULL);
    g_io_channel_set_buffered(channel, FALSE);

    g_io_add_watch(
        channel,
        G_IO_IN | G_IO_HUP,
        (GIOFunc) cb_io_watch_pty,
        term
    );

    g_io_channel_unref(channel);

    // Try to set the font to the OEM 8x16 one
    //
    PangoFontDescription* desc =
        pango_font_description_from_string("TerminalVector 9");

    vte_terminal_set_font(VTE_TERMINAL(term), desc);

    pango_font_description_free(desc);

    // Print default str
    //
    gchar* motd =
        g_strdup_printf(
            "%s [Version %s.%s.%s]\r\n"
            "(C) Copyright 1985-2005 Microsoft Corp\r\n\n",
            wintc_build_query(WINTC_VER_NAME),
            wintc_build_query(WINTC_VER_MAJOR),
            wintc_build_query(WINTC_VER_MINOR),
            wintc_build_query(WINTC_VER_BUILD)
        );

    vte_terminal_feed(VTE_TERMINAL(term), motd, -1);

    g_free(motd);

    // Launch the user's shell
    //
    static gchar* s_cmd[] = {
        "/usr/bin/env",
        NULL, // $SHELL
        NULL
    };

    if (!s_cmd[1])
    {
        s_cmd[1] = g_strdup(g_getenv("SHELL"));
    }

    vte_terminal_set_pty(VTE_TERMINAL(term), self->pty);

    vte_pty_spawn_async(
        self->pty,
        g_get_home_dir(),
        s_cmd,
        NULL,
        G_SPAWN_DEFAULT,
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        (GAsyncReadyCallback) cb_spawn_pty,
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cmd_window_dispose(
    GObject* object
)
{
    WinTCCmdWindow* wnd = WINTC_CMD_WINDOW(object);

    g_clear_object(&(wnd->pty));

    (G_OBJECT_CLASS(wintc_cmd_window_parent_class))
        ->dispose(object);
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

//
// CALLBACKS
//
static gboolean cb_io_watch_pty(
    GIOChannel*  source,
    GIOCondition condition,
    gpointer     user_data
)
{
    VteTerminal* term = VTE_TERMINAL(user_data);

    if (condition & G_IO_HUP)
    {
        return FALSE;
    }

    // Read through data
    //
    gchar   buf[4096];
    GError* error = NULL;
    gsize   n_read = 0;

    GIOStatus status =
        g_io_channel_read_chars(
            source,
            buf,
            4096,
            &n_read,
            &error
        );

    if (status == G_IO_STATUS_NORMAL && n_read > 0)
    {
        gchar* buf_nofmt = g_malloc(n_read);
        gsize  i_src     = 0;
        gsize  i_dst     = 0;

        while (i_src < n_read)
        {
            // Handle sequences
            //
            if (
                buf[i_src] == '\x1b' &&
                i_src + 1 < n_read   &&
                buf[i_src + 1] == '['
            )
            {
                // Find the terminator for the sequence, we're looking for the
                // 'm' command for formatting but we don't know what this
                // escape sequence is yet 'til we check the terminator
                //
                gsize m_idx;

                for (m_idx = i_src + 2; m_idx < n_read; m_idx++)
                {
                    if (buf[m_idx] >= 0x40 && buf[m_idx] <= 0x7E)
                    {
                        break;
                    }
                }

                // Didn't find it? bail!
                //
                // FIXME: This could fudge things up if an escape sequence
                //        gets split across buffers...?
                //
                if (m_idx == n_read)
                {
                    break;
                }

                // Did find it, handle if it is a formatting sequence
                //
                if (buf[m_idx] == 'm')
                {
                    gchar*  sequence = wintc_substr(
                                           &buf[i_src + 2],
                                           &buf[m_idx]
                                       );
                    gchar** codes    = g_strsplit(sequence, ";", -1);

                    gboolean is_bold = FALSE;

                    // We have to do two passes to check if there is even a
                    // 'bold' format in this sequence - if there isn't one
                    // then we need to skip the whole thing
                    //
                    for (gchar** code = codes; *code; code++)
                    {
                        if (atol(*code) == 1)
                        {
                            is_bold = TRUE;
                        }
                    }

                    // If it was bold we need to handle it, otherwise just continue
                    // to printing out as normal
                    //
                    if (is_bold)
                    {
                        gsize len_seq = strlen(sequence) + 3; // + \x1b[m

                        // If bold was not the only sequence we still need to output the
                        // rest
                        //
                        if (*(codes + 1))
                        {
                            buf_nofmt[i_dst++] = '\x1b';
                            buf_nofmt[i_dst++] = '[';

                            for (gchar** code = codes; *code; code++)
                            {
                                if (atol(*code) == 1)
                                {
                                    continue;
                                }

                                // Remember to insert semicolon before sequences
                                // other than the first
                                //
                                if (buf_nofmt[i_dst - 1] != '[')
                                {
                                    buf_nofmt[i_dst++] = ';';
                                }

                                // Copy into buffer
                                //
                                gsize len = strlen(*code);

                                memcpy(&buf_nofmt[i_dst], *code, len);
                                i_dst += len;
                            }

                            buf_nofmt[i_dst++] = 'm';
                        }

                        i_src += len_seq;
                    }

                    g_strfreev(codes);
                    g_free(sequence);

                    // Fall-through to simple copy if it was any other sequence
                    //
                    if (is_bold) { continue; }
                }
            }

            buf_nofmt[i_dst++] = buf[i_src++];
        }

        vte_terminal_feed(term, buf_nofmt, i_dst);

        g_free(buf_nofmt);
    }

    if (error)
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    return TRUE;
}

static void cb_spawn_pty(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCCmdWindow* wnd = WINTC_CMD_WINDOW(user_data);

    GError* error = NULL;

    if (
        !vte_pty_spawn_finish(
            VTE_PTY(source_object),
            res,
            &(wnd->pid_shell),
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);

        // If mapped we should destroy now, otherwise set PID to -1 so it'll
        // destroy on map-event
        //
        wnd->pid_shell = -1;

        if (gtk_widget_get_mapped(GTK_WIDGET(wnd)))
        {
            gtk_window_close(GTK_WINDOW(wnd));
        }

        return;
    }

    // Watch the PID to kill the window when exited
    //
    g_child_watch_add(
        wnd->pid_shell,
        (GChildWatchFunc) cb_watch_pid_shell,
        wnd
    );
}

static void cb_watch_pid_shell(
    WINTC_UNUSED(GPid pid),
    WINTC_UNUSED(gint wait_status),
    gpointer user_data
)
{
    WinTCCmdWindow* wnd = WINTC_CMD_WINDOW(user_data);

    // Just exit
    //
    gtk_window_close(GTK_WINDOW(wnd));
}

static void on_window_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEvent* event),
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCCmdWindow* wnd = WINTC_CMD_WINDOW(self);

    if (wnd->pid_shell < 0)
    {
        gtk_window_close(GTK_WINDOW(wnd));
    }
}
