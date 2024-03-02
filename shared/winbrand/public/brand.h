/** @file */

#ifndef __WINBRAND_BRAND_H__
#define __WINBRAND_BRAND_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

//
// PUBLIC ENUMS
//

/**
 * Specifies branding parts available in WinTC.
 */
typedef enum
{
    /** The short SKU branding banner. */
    WINTC_BRAND_PART_BANNER       = 0,
    /** The tall SKU branding banner. */
    WINTC_BRAND_PART_BANNER_TALL,
    /** The static SKU branding strip. */
    WINTC_BRAND_PART_STRIP_STATIC = 10,
    /** The animated SKU branding strip. */
    WINTC_BRAND_PART_STRIP_ANIM
} WinTCBrandPart;

//
// PUBLIC FUNCTIONS
//

/**
 * Retrieves a pixmap for the specified branding part.
 *
 * @param part  The desired branding part.
 * @param error Storage location for any error that occurred.
 * @return The requested pixmap, if successful.
 */
GdkPixbuf* wintc_brand_get_brand_pixmap(
    WinTCBrandPart part,
    GError**       error
);

#endif
