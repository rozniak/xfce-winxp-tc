#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <glib.h>

void wintc_display_error_and_clear(
    GError** error
);

void wintc_log_error_and_clear(
    GError** error
);

#endif
