
#ifndef WINTC_WELCOME_BALLOON_CONTAINER_H
#define WINTC_WELCOME_BALLOON_CONTAINER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WELCOME_BALLOON (wintc_welcome_balloon_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWelcomeBalloon,
    wintc_welcome_balloon,
    WINTC,
    WELCOME_BALLOON,
    GtkBox
)

//
// PUBLIC ENUMS
//
typedef enum {
    BALLOON_TYPE_ERROR,
    BALLOON_TYPE_WARNING,
} BalloonType;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_balloon_new_with_type(BalloonType type, GtkWidget* target_widget);

G_END_DECLS

#endif