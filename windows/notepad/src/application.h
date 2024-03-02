#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotepadApplicationClass WinTCNotepadApplicationClass;
typedef struct _WinTCNotepadApplication      WinTCNotepadApplication;

#define WINTC_TYPE_NOTEPAD_APPLICATION            (wintc_notepad_application_get_type())
#define WINTC_NOTEPAD_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTEPAD_APPLICATION, WinTCNotepadApplication))
#define WINTC_NOTEPAD_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTEPAD_APPLICATION, WinTCNotepadApplicationClass))
#define IS_WINTC_NOTEPAD_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTEPAD_APPLICATION))
#define IS_WINTC_NOTEPAD_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTEPAD_APPLICATION))
#define WINTC_NOTEPAD_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTEPAD_APPLICATION, WinTCNotepadApplicationClass))

GType wintc_notepad_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCNotepadApplication* wintc_notepad_application_new(void);

#endif
