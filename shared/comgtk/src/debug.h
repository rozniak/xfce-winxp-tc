#ifndef __DEBUG_H__
#define __DEBUG_H__

#define WINTC_ENVVAR_DEBUG_LOGGING "WINDEBUG"
#define WINTC_LOG_DEBUG(...) if (getenv(WINTC_ENVVAR_DEBUG_LOGGING)) { g_message(__VA_ARGS__); }

#endif
