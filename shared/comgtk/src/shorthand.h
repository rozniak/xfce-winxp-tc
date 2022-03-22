#ifndef __SHORTHAND_H__
#define __SHORTHAND_H__

#define WINTC_SAFE_REF_CLEAR(ref) if (ref != NULL) { *ref = NULL; }
#define WINTC_SAFE_REF_SET(ref, value) if (ref != NULL) { *ref = value; }

#define WINTC_UNUSED(arg) __attribute__((unused)) arg

#endif
