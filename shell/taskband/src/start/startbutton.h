#ifndef __STARTBUTTON_H__
#define __STARTBUTTON_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "startmenu.h"

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _StartButtonPrivate StartButtonPrivate;
typedef struct _StartButtonClass   StartButtonClass;
typedef struct _StartButton        StartButton;

#define TYPE_START_BUTTON            (start_button_get_type())
#define START_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_START_BUTTON, StartButton))
#define START_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_START_BUTTON, StartButtonClass))
#define IS_START_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_START_BUTTON))
#define IS_START_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_START_BUTTON))
#define START_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_START_BUTTON, StartButtonClass))

GType start_button_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* start_button_new(void);

G_END_DECLS

#endif
