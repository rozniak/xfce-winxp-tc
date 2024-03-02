/** @file */

#ifndef __MSGINA_CHALLENGE_H__
#define __MSGINA_CHALLENGE_H__

/**
 * Specifies challenge response outcomes from GINA.
 */
typedef enum
{
    /** The authetication attempt was successful. */
    WINTC_GINA_RESPONSE_OKAY = 0,
    /** The authentication attempt was a failure. */
    WINTC_GINA_RESPONSE_FAIL = 1
} WinTCGinaResponse;

#endif

