#ifndef __COMCTL_CPL_H__
#define __COMCTL_CPL_H__

//
// PUBLIC FUNCTIONS
//
void wintc_ctl_cpl_notebook_append_page_from_resource(
    GtkNotebook* notebook,
    const gchar* resource_path,
    ...
);

#endif
