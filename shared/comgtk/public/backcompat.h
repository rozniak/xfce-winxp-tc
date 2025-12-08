#ifndef __COMGTK_BACKCOMPAT_H__
#define __COMGTK_BACKCOMPAT_H__

//
// This file is for backporting newer GLib/GTK bits and bobs if necessary for
// building the project on older distros
//

//
// Backport G_CONNECT_DEFAULT
//
#ifndef GLIB_VERSION_2_74
#define G_CONNECT_DEFAULT ((GConnectFlags) 0)
#endif

#endif
