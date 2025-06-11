
#ifndef BALLOON_CONTAINER_H
#define BALLOON_CONTAINER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BALLOON_TYPE_CONTAINER (balloon_get_type())
G_DECLARE_FINAL_TYPE(Balloon, balloon, BALLOON, WIDGET, GtkWindow)

struct _BalloonClass {
    GtkWindowClass parent_class;
};

typedef enum {
    BALLOON_TYPE_ERROR,
    BALLOON_TYPE_WARNING,
} BalloonType;

#define BALLOON_TYPE (balloon_type_get_type())
GType balloon_type_get_type(void) G_GNUC_CONST;

GtkWidget* balloon_new(BalloonType type, GtkWidget* target_widget);

G_END_DECLS

#endif