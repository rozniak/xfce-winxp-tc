#include "config.h"

#include <glib.h>
#include <glib/gi18n-lib.h>

#include "places.h"

//
// STATIC VARIABLES
//
static gchar* place_texts[] = {
    N_("Application Data"),
    N_("Desktop"),
    N_("Downloads"),
    N_("Favorites"),
    N_("My Documents"),
    N_("My Music"),
    N_("My Pictures"),
    N_("My Recent Documents"),
    N_("Recycle Bin"),
    N_("My Video"),
    N_("Adminstrative Tools"),
    N_("My Computer"),
    N_("My Network Places"),
    N_("Control Panel"),
    N_("Network Connections"),
    N_("Printers and Faxes")
};

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_get_place_name(
    WinTCShellPlace place
)
{
    return _(place_texts[place]);
}
