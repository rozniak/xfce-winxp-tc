#ifndef __STRIPCTL_H__
#define __STRIPCTL_H__

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCGinaStripClass WinTCGinaStripClass;
typedef struct _WinTCGinaStrip      WinTCGinaStrip;

#define TYPE_WINTC_GINA_STRIP            (wintc_gina_strip_get_type())
#define WINTC_GINA_STRIP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_GINA_STRIP, WinTCGinaStrip))
#define WINTC_GINA_STRIP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_GINA_STRIP, WinTCGinaStrip))
#define IS_WINTC_GINA_STRIP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_GINA_STRIP))
#define IS_WINTC_GINA_STRIP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_GINA_STRIP))
#define WINTC_GINA_STRIP_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), TYPE_WINTC_GINA_STRIP))

GType wintc_gina_strip_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_strip_new(void);

void wintc_gina_strip_animate(
    WinTCGinaStrip* strip
);
void wintc_gina_strip_stop_animating(
    WinTCGinaStrip* strip
);

#endif

