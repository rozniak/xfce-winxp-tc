#ifndef __START_MFU_H__
#define __START_MFU_H__

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_START_MFU_TRACKER (wintc_start_mfu_tracker_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCStartMfuTracker,
    wintc_start_mfu_tracker,
    WINTC,
    START_MFU_TRACKER,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCStartMfuTracker* wintc_start_mfu_tracker_get_default(void);

void wintc_start_mfu_tracker_bump_cmdline(
    WinTCStartMfuTracker* mfu_tracker,
    const gchar*          cmdline
);
GList* wintc_start_mfu_tracker_get_mfu_list(
    WinTCStartMfuTracker* mfu_tracker
);

#endif
