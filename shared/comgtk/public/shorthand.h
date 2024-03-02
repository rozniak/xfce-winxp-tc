/** @file */

#ifndef __COMGTK_SHORTHAND_H__
#define __COMGTK_SHORTHAND_H__

/**
 * @def WINTC_SAFE_REF_CLEAR(ref)
 *
 * Sets the value pointed at by ref to NULL, checks ref itself is not NULL. 
 */
#define WINTC_SAFE_REF_CLEAR(ref) if (ref != NULL) { *ref = NULL; }

/**
 * @def WINTC_SAFE_REF_SET(ref)
 *
 * Sets the value pointed at by ref, checks ref itself is not NULL.
 */
#define WINTC_SAFE_REF_SET(ref, value) if (ref != NULL) { *ref = value; }

/**
 * @def WINTC_UNUSED(arg)
 *
 * Marks a parameter as unused to the compiler so it doesn't complain.
 */
#define WINTC_UNUSED(arg) __attribute__((unused)) arg

#endif
