#ifndef __WINTC_WINBRAND_H__
#define __WINTC_WINBRAND_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

//
// Branding banners
//
GdkPixbuf* wintc_brand_get_banner(
    GError** error
);

GdkPixbuf* wintc_brand_get_progress_strip(
    GError** error
);

#endif
