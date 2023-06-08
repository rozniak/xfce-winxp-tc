#ifndef __PROGRAMSLIST_H__
#define __PROGRAMSLIST_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _ProgramsListClass ProgramsListClass;
typedef struct _ProgramsList      ProgramsList;

#define TYPE_PROGRAMS_LIST            (programs_list_get_type())
#define PROGRAMS_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_PROGRAMS_LIST, ProgramsList))
#define PROGRAMS_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_PROGRAMS_LIST, ProgramsListClass))
#define IS_PROGRAMS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_PROGRAMS_LIST))
#define IS_PROGRAMS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_PROGRAMS_LIST))
#define PROGRAMS_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_PROGRAMS_LIST, ProgramsListClass))

GType programs_list_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
void programs_list_refresh(
    ProgramsList* programs_list
);

G_END_DECLS

#endif
