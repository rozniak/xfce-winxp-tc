#ifndef __BRAND_H__
#define __BRAND_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

typedef enum
{
    WINTC_BRAND_PART_BANNER       = 0,
    WINTC_BRAND_PART_BANNER_TALL,
    WINTC_BRAND_PART_STRIP_STATIC = 10,
    WINTC_BRAND_PART_STRIP_ANIM
} WinTCBrandPart;

GdkPixbuf* wintc_brand_get_brand_pixmap(
    WinTCBrandPart part,
    GError**       error
);

#endif
