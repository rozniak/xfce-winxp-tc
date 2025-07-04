
#ifndef BALLOON_CONTAINER_H
#define BALLOON_CONTAINER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _BalloonClass BalloonClass;
typedef struct _Balloon      Balloon;

#define BALLOON_TYPE_CONTAINER            (balloon_get_type())
#define BALLOON(obj)                      (G_TYPE_CHECK_INSTANCE_CAST((obj), BALLOON_TYPE_CONTAINER, Balloon))
#define BALLOON_CLASS(klass)              (G_TYPE_CHECK_CLASS_CAST((klass), BALLOON_TYPE_CONTAINER, BalloonClass))
#define IS_BALLOON(obj)                   (G_TYPE_CHECK_INSTANCE_TYPE((obj), BALLOON_TYPE_CONTAINER))
#define IS_BALLOON_CLASS(klass)           (G_TYPE_CHECK_CLASS_TYPE((klass), BALLOON_TYPE_CONTAINER))
#define BALLOON_GET_CLASS(obj)            (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), BALLOON_TYPE_CONTAINER, BalloonClass))

GType balloon_get_type(void) G_GNUC_CONST;

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
GtkWidget* balloon_new(BalloonType type, GtkWidget* target_widget);

G_END_DECLS

#endif