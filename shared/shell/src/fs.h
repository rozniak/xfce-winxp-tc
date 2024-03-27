#ifndef __FS_H__
#define __FS_H__

#include <glib.h>

//
// FIXME: Temp! fs.c/h should be moved to another lib!
//
//        For now we just have sync I/O here, we need:
//        - Should move to async I/O
//        - Move FS stuff to a separate lib
//        - MIME type detection
//        - Symlink/shortcut handling
//

//
// PUBLIC FUNCTIONS
//
GSList* wintc_sh_fs_get_names_as_list(
    const gchar* path,
    gboolean     full_names,
    GError**     error
);

#endif
