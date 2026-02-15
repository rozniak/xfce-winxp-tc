#ifndef __PHASE_H__
#define __PHASE_H__

#include <glib.h>

//
// PUBLIC ENUMS
//
typedef enum _WinTCSetupPhase
{
    WINTC_SETUP_PHASE_TEXTMODE,
    WINTC_SETUP_PHASE_GUIMODE,
    WINTC_SETUP_PHASE_OOBE,
    WINTC_SETUP_PHASE_COMPLETE
} WinTCSetupPhase;

//
// PUBLIC FUNCTIONS
//
gboolean wintc_setup_phase_set(
    WinTCSetupPhase phase
);

#endif
