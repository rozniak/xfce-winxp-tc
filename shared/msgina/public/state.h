/** @file */

#ifndef __MSGINA_STATE_H__
#define __MSGINA_STATE_H__

/**
 * Specifies the possible states of a GINA logon provider.
 */
typedef enum
{
    /** The provider has not even begun. */
    WINTC_GINA_STATE_NONE = 0,
    /** The provider is in the process of starting up. */
    WINTC_GINA_STATE_STARTING,
    /** The provider has displayed the user prompt and is awaiting input. */
    WINTC_GINA_STATE_PROMPT,
    /** The provider is in the process of launching the user session. */
    WINTC_GINA_STATE_LAUNCHING
} WinTCGinaState;

#endif

