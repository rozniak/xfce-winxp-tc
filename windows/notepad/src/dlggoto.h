#ifndef __DLGGOTO_H__
#define __DLGGOTO_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTEPAD_GO_TO_DIALOG (wintc_notepad_go_to_dialog_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotepadGoToDialog,
    wintc_notepad_go_to_dialog,
    WINTC,
    NOTEPAD_GO_TO_DIALOG,
    GtkWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notepad_go_to_dialog_new(void);

gint wintc_notepad_go_to_dialog_get_response(
    WinTCNotepadGoToDialog* dlg_goto
);
gint wintc_notepad_go_to_dialog_get_line_number(
    WinTCNotepadGoToDialog* dlg_goto
);
gint wintc_notepad_go_to_dialog_get_max_line_number(
    WinTCNotepadGoToDialog* dlg_goto
);
void wintc_notepad_go_to_dialog_set_line_number(
    WinTCNotepadGoToDialog* dlg_goto,
    gint                    line_number
);
void wintc_notepad_go_to_dialog_set_max_line_number(
    WinTCNotepadGoToDialog* dlg_goto,
    gint                    max_line
);

#endif
