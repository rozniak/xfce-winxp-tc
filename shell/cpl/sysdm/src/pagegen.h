#ifndef __PAGEGEN_H__
#define __PAGEGEN_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_CPL_SYSDM_PAGE_GENERAL (wintc_cpl_sysdm_page_general_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCCplSysdmPageGeneral,
    wintc_cpl_sysdm_page_general,
    WINTC,
    CPL_SYSDM_PAGE_GENERAL,
    WinTCShextUIController
)

#endif
