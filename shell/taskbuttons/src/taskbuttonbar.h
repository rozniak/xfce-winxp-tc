#ifndef __TASKBUTTONBAR_H__
#define __TASKBUTTONBAR_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _TaskButtonBarPrivate TaskButtonBarPrivate;
typedef struct _TaskButtonBarClass   TaskButtonBarClass;
typedef struct _TaskButtonBar        TaskButtonBar;

#define TYPE_TASKBUTTON_BAR            (taskbutton_bar_get_type())
#define TASKBUTTON_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_TASKBUTTON_BAR, TaskButtonBar))
#define TASKBUTTON_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_TASKBUTTON_BAR, TaskButtonBarClass))
#define IS_TASKBUTTON_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_TASKBUTTON_BAR))
#define IS_TASKBUTTON_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_TASKBUTTON_BAR))
#define TASKBUTTON_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_TASKBUTTON_BAR, TaskButtonBarClass))

GType taskbutton_bar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* taskbutton_bar_new(void);

G_END_DECLS

#endif
