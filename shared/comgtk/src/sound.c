#include <canberra.h>
#include <canberra-gtk.h>
#include <glib.h>

#include "../public/sound.h"

//
// PUBLIC FUNCTIONS
//
void wintc_xdg_play_sound(
    const gchar* xdg_name
)
{
    ca_context* ctx = ca_gtk_context_get();

    ca_context_play(
        ctx,
        0,
        CA_PROP_EVENT_ID, xdg_name,
        NULL
    );
}
