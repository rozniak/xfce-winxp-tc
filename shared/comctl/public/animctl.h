/** @file */

#ifndef __COMCTL_ANIMCTL_H__
#define __COMCTL_ANIMCTL_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC DEFINES
//

/**
 * @def WINTC_CTL_ANIMATION_INFINITE
 *
 * Specifies that the animation should be looped indefinitely.
 */
#define WINTC_CTL_ANIMATION_INFINITE 0

/**
 * @def WINTC_CTL_ANIMATION_NONE
 *
 * Represents no animation.
 */
#define WINTC_CTL_ANIMATION_NONE 0

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCtlAnimationClass WinTCCtlAnimationClass;

/**
 * A WinTC animation control.
 */
typedef struct _WinTCCtlAnimation WinTCCtlAnimation;

#define WINTC_TYPE_CTL_ANIMATION            (wintc_ctl_animation_get_type())
#define WINTC_CTL_ANIMATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CTL_ANIMATION, WinTCCtlAnimation))
#define WINTC_CTL_ANIMATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CTL_ANIMATION, WinTCCtlAnimation))
#define IS_WINTC_CTL_ANIMATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CTL_ANIMATION))
#define IS_WINTC_CTL_ANIMATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CTL_ANIMATION))
#define WINTC_CTL_ANIMATION_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CTL_ANIMATION))

GType wintc_ctl_animation_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a new instance of WinTCCtlAnimation.
 *
 * @return The new WinTCCtlAnimation instance cast to GtkWidget.
 */
GtkWidget* wintc_ctl_animation_new(void);

/**
 * Adds a framesheet to an animation control.
 *
 * @param anim              The animation control.
 * @param framesheet_pixbuf The framesheet.
 * @param frame_count       The number of frames in the framesheet.
 * @return The ID to reference the framesheet in the animation control.
 */
guint wintc_ctl_animation_add_framesheet(
    WinTCCtlAnimation* anim,
    GdkPixbuf*         framesheet_pixbuf,
    gint               frame_count
);

/**
 * Adds a static frame to an animation control.
 *
 * @param anim          The animation control.
 * @param static_pixbuf The single frame.
 * @return The ID to reference the frame in the animation control.
 */
guint wintc_ctl_animation_add_static(
    WinTCCtlAnimation* anim,
    GdkPixbuf*         static_pixbuf
);

/**
 * Retrieves the number of animations in an animation control.
 *
 * @param anim The animation control.
 */
guint wintc_ctl_animation_get_count(
    WinTCCtlAnimation* anim
);

/**
 * Retrieves the horizontal alignment property of an animation control.
 *
 * @param anim The animation control.
 * @return The current horizontal alignment of the animation control.
 */
GtkAlign wintc_ctl_animation_get_halign(
    WinTCCtlAnimation* anim
);

/**
 * Retrieves the vertical alignment property of an animation control.
 *
 * @param anim The animation control.
 * @return The current vertical alignment of the animation control.
 */
GtkAlign wintc_ctl_animation_get_valign(
    WinTCCtlAnimation* anim
);

/**
 * Plays an animation in an animation control.
 *
 * @param anim       The animation control.
 * @param id         The ID of the animation to play.
 * @param frame_rate The desired frame rate for playback.
 * @param repeats    The number of times to repeat the animation.
 */
void wintc_ctl_animation_play(
    WinTCCtlAnimation* anim,
    guint              id,
    gint               frame_rate,
    gint               repeats
);

/**
 * Removes an animation from an animation control.
 *
 * @param anim The animation control.
 * @param id   The ID of the animation to remove.
 */
void wintc_ctl_animation_remove(
    WinTCCtlAnimation* anim,
    guint              id
);

/**
 * Sets the horizontal alignment property of an animation control.
 *
 * @param anim  The animation control.
 * @param align The desired horizontal alignment.
 */
void wintc_ctl_animation_set_halign(
    WinTCCtlAnimation* anim,
    GtkAlign           align
);

/**
 * Sets the vertical alignment property of an animation control.
 *
 * @param anim  The animation control.
 * @param align The desired vertical alignment.
 */
void wintc_ctl_animation_set_valign(
    WinTCCtlAnimation* anim,
    GtkAlign           align
);

#endif

