#ifndef __DEPLOY_H__
#define __DEPLOY_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
gboolean wintc_oobe_deploy_drop_file(
    const gchar* resource_path,
    const gchar* drop_directory,
    const gchar* drop_filename,
    GError** error
);

#endif
