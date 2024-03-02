#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "../public/controls.h"
#include "../public/punctuation.h"

//
// STATIC VARIABLES
//
static const gchar* control_texts[] = {
    // Standard dialog buttons
    //
    N_("OK"),
    N_("Cancel"),
    N_("Yes"),
    N_("No"),
    N_("Abort"),
    N_("Retry"),
    N_("Ignore"),
    N_("Help"),

    // Menus
    //
    N_("File"),
    N_("Edit"),
    N_("View"),
    N_("Insert"),
    N_("Format"),
    N_("Tools"),
    N_("Window"),
    N_("Help"),

    // File menus
    //
    N_("New"),
    N_("Browse"),
    N_("Open"),
    N_("Save"),
    N_("Save As"),
    N_("Save All"),
    N_("Close"),
    N_("Close All"),
    N_("Properties"),

    N_("Print"),
    N_("Print Preview"),
    N_("Page Setup"),

    N_("Exit"),

    // Edit menus
    //
    N_("Undo"),
    N_("Redo"),

    N_("Cut"),
    N_("Copy"),
    N_("Paste"),
    N_("Delete"),

    N_("Find"),
    N_("Find Next"),
    N_("Replace"),
    N_("Replace All"),
    N_("Go To"),

    N_("Select All"),

    // View menus
    //
    N_("Zoom"),
    N_("Zoom In"),
    N_("Zoom Out"),
    N_("Best Fit"),
    N_("Actual Size"),
    N_("Full Screen"),
    N_("Status Bar"),

    // Format menus
    //
    N_("Font"),
    N_("Font style"),
    N_("Bold"),
    N_("Italic"),
    N_("Regular"),
    N_("Size"),
    N_("Word Wrap"),

    // Tools menus
    //
    N_("Options"),
    N_("Customize"),

    // Window menus
    //
    N_("New Window"),
    N_("New Tab"),
    N_("Close Window"),
    N_("Close Tab"),

    // Help menus
    //
    N_("Help Topics"),
    N_("About %s")
};

static const gchar* punctuation_texts[] = {
    "",        // No punctuation
    N_("..."), // More input
    N_(":")    // Itemization
};

static const gchar* generated_texts[sizeof(punctuation_texts)][sizeof(control_texts)];

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_lc_get_control_text(
    WinTCControlTextId text_id,
    WinTCPunctuationId punc_id
)
{
    // If punctuation is needed - we must generate a string now, though this code
    // assumes a typical 'English' layout with the punctuation at the end, this may not
    // be true for all locales but I figure we'll cross that bridge if/when we get
    // there
    //
    if (punc_id)
    {
        // Check if we have generated a string already, if so, return that
        //
        if (generated_texts[punc_id][text_id])
        {
            return generated_texts[punc_id][text_id];
        }

        // We must generate one now, store it if needed again later, and hand it off
        //
        const gchar* new_string =
            g_strdup_printf(
                "%s%s",
                _(control_texts[text_id]),
                _(punctuation_texts[punc_id])
            );

        generated_texts[punc_id][text_id] = new_string;

        return new_string;
    }

    // No punctuation needed so we can just return the translated string simple as
    //
    return _(control_texts[text_id]);
}
