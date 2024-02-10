#ifndef __ANIMCTL_H__
#define __ANIMCTL_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC DEFINES
//
#define WINTC_ANIMATION_INFINITE 0
#define WINTC_ANIMATION_NONE     0

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCAnimationPrivate WinTCAnimationPrivate;
typedef struct _WinTCAnimationClass   WinTCAnimationClass;
typedef struct _WinTCAnimation        WinTCAnimation;

#define TYPE_WINTC_ANIMATION            (wintc_animation_get_type())
#define WINTC_ANIMATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_ANIMATION, WinTCAnimation))
#define WINTC_ANIMATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_ANIMATION, WinTCAnimation))
#define IS_WINTC_ANIMATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_ANIMATION))
#define IS_WINTC_ANIMATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_ANIMATION))
#define WINTC_ANIMATION_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), TYPE_WINTC_ANIMATION))

GType wintc_animation_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_animation_new(void);

guint wintc_animation_add_framesheet(
    WinTCAnimation* anim,
    GdkPixbuf*      framesheet_pixbuf,
    gint            frame_count
);
guint wintc_animation_add_static(
    WinTCAnimation* anim,
    GdkPixbuf*      static_pixbuf
);
guint wintc_animation_get_count(
    WinTCAnimation* anim
);
GtkAlign wintc_animation_get_halign(
    WinTCAnimation* anim
);
GtkAlign wintc_animation_get_valign(
    WinTCAnimation* anim
);
void wintc_animation_play(
    WinTCAnimation* anim,
    guint           id,
    gint            frame_rate,
    gint            repeats
);
void wintc_animation_remove(
    WinTCAnimation* anim,
    guint           id
);
void wintc_animation_set_halign(
    WinTCAnimation* anim,
    GtkAlign        align
);
void wintc_animation_set_valign(
    WinTCAnimation* anim,
    GtkAlign        align
);

#endif

