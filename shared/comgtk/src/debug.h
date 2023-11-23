#ifndef __DEBUG_H__
#define __DEBUG_H__

// Two variants for debug messaging are provided:
//     USER_DEBUG is at runtime, intended at end user debugging
//     DEBUG is compile time, intended for development
//
#define WINTC_ENVVAR_DEBUG_LOGGING "WINDEBUG"
#define WINTC_LOG_USER_DEBUG(...) if (getenv(WINTC_ENVVAR_DEBUG_LOGGING)) { g_message(__VA_ARGS__); }

#ifdef WINTC_CHECKED
#define WINTC_LOG_DEBUG(...) g_message(__VA_ARGS__);
#else
#define WINTC_LOG_DEBUG(...)
#endif

#endif