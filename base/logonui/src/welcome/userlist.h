#ifndef __USERLIST_H__
#define __USERLIST_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/msgina.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WELCOME_USER_LIST            (wintc_welcome_user_list_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWelcomeUserList,
    wintc_welcome_user_list,
    WINTC,
    WELCOME_USER_LIST,
    GtkContainer
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_user_list_new(
    WinTCGinaLogonSession* logon_session
);

#endif
