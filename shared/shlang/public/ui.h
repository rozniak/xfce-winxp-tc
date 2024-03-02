/** @file */

#ifndef __SHLANG_UI_H__
#define __SHLANG_UI_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Locates and translates any known placeholders in a GtkBuilder's widget tree.
 *
 * @param builder The GtkBuilder instance.
 */
void wintc_lc_builder_preprocess_widget_text(
    GtkBuilder* builder
);

#endif
