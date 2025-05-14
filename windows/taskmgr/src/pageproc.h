#ifndef __PAGEPROC_H__
#define __PAGEPROC_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TASKMGR_PAGE_PROCESSES (wintc_taskmgr_page_processes_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCTaskmgrPageProcesses,
    wintc_taskmgr_page_processes,
    WINTC,
    TASKMGR_PAGE_PROCESSES,
    WinTCShextUIController
)

#endif
