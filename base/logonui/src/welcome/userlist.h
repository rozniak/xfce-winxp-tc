#ifndef __USERLIST_H__
#define __USERLIST_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/msgina.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCWelcomeUserListClass WinTCWelcomeUserListClass;
typedef struct _WinTCWelcomeUserList      WinTCWelcomeUserList;

#define WINTC_TYPE_WELCOME_USER_LIST            (wintc_welcome_user_list_get_type())
#define WINTC_WELCOME_USER_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_WELCOME_USER_LIST, WinTCWelcomeUserList))
#define WINTC_WELCOME_USER_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_WELCOME_USER_LIST, WinTCWelcomeUserList))
#define IS_WINTC_WELCOME_USER_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_WELCOME_USER_LIST))
#define IS_WINTC_WELCOME_USER_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_WELCOME_USER_LIST))
#define WINTC_WELCOME_USER_LIST_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_WELCOME_USER_LIST))

GType wintc_welcome_user_list_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_user_list_new(
    WinTCGinaLogonSession* logon_session
);

#endif
