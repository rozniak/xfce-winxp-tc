/** @file */

#ifndef __COMGTK_DEBUG_H__
#define __COMGTK_DEBUG_H__

#include <glib.h>
#include <stdlib.h>

/**
 * @def WINTC_ENVVAR_DEBUG_LOGGING
 *
 * The environment variable that, if set, will enable troubleshooting log
 * output.
 */
#define WINTC_ENVVAR_DEBUG_LOGGING "WINDEBUG"

/**
 * @def WINTC_LOG_USER_DEBUG(...)
 *
 * Logs a message to stdout if the WINDEBUG environment variable is set, this
 * is intended for messages that will help end users troubleshoot a problem.
 */
#define WINTC_LOG_USER_DEBUG(...) if (getenv(WINTC_ENVVAR_DEBUG_LOGGING)) { g_message(__VA_ARGS__); }

/**
 * @def WINTC_LOG_DEBUG(...)
 *
 * Logs a message to stdout if compiled as a checked build, this is intended
 * for messages that will aid in developer debugging.
 */
#ifdef WINTC_CHECKED
#define WINTC_LOG_DEBUG(...) g_message(__VA_ARGS__);
#else
#define WINTC_LOG_DEBUG(...)
#endif

#endif

