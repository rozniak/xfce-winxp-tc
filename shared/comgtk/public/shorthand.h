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

/**
 * @def WINTC_DEBUG_ONLY(arg)
 *
 * Marks a parameter as only used in chcked builds, so the compiler does not
 * complain for free builds.
 */
#ifdef WINTC_CHECKED
#define WINTC_DEBUG_ONLY(arg) arg
#else
#define WINTC_DEBUG_ONLY(arg) WINTC_UNUSED(arg)
#endif

/**
 * @def WINTC_RETURN_IF_FAIL(cond)
 *
 * Returns from a function if the condition fails - this is provided as an
 * alternative to g_return_if_fail so that G_DISABLE_CHECKS does not apply.
 */
#define WINTC_RETURN_IF_FAIL(cond) if (!(cond)) { return; }

/**
 * @def WINTC_RETURN_VAL_IF_FAIL(cond, ret)
 *
 * Returns a value from a function if the condition fails - this is provided
 * as an alternative to g_return_val_if_fail so that G_DISABLED_CHECKS does
 * not apply.
 */
#define WINTC_RETURN_VAL_IF_FAIL(cond, ret) if (!(cond)) { return (ret); }

#endif
