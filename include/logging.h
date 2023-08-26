#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

enum LOG_LEVEL {LEVEL_ERROR, LEVEL_WARN, LEVEL_INFO, LEVEL_DEBUG};

extern void log_log(enum LOG_LEVEL log_level, const char *msg, ...);

#define log_error(...) log_log(LEVEL_ERROR, __VA_ARGS__)
#define log_warn(...) log_log(LEVEL_WARN, __VA_ARGS__)
#define log_info(...) log_log(LEVEL_INFO, __VA_ARGS__)
#define log_debug(...) log_log(LEVEL_DEBUG, __VA_ARGS__)

#endif