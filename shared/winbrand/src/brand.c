#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/brand.h"

//
// PUBLIC FUNCTIONS
//
GdkPixbuf* wintc_brand_get_brand_pixmap(
    WinTCBrandPart part,
    GError**       error
)
{
    gchar* path = NULL;

    switch (part)
    {
        case WINTC_BRAND_PART_BANNER:
            path = WINTC_ASSETS_DIR "/brand/banner.png";
            break;

        case WINTC_BRAND_PART_BANNER_TALL:
            path = WINTC_ASSETS_DIR "/brand/bannerx.png";
            break;

        case WINTC_BRAND_PART_STRIP_STATIC:
            path = WINTC_ASSETS_DIR "/brand/strip-static.png";
            break;

        case WINTC_BRAND_PART_STRIP_ANIM:
            path = WINTC_ASSETS_DIR "/brand/strip-anim.png";
            break;

        default:
            g_critical("Unknown brand part %d", part);
            return NULL;
    }

    return gdk_pixbuf_new_from_file(path, error);
}
