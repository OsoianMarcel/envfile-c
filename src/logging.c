#include "logging.h"

static const char *level_strings[] = {
  "ERROR", "WARN", "INFO", "DEBUG"
};

static char *log_time()
{
    time_t currentTime;
    time(&currentTime);

    char* str_time = ctime(&currentTime);
    // Remove the new line char.
    str_time[strlen(str_time) - 1] = '\0';

    return str_time;
}

void log_log(enum LOG_LEVEL log_level, const char *msg, ...) {
    FILE *stream = log_level == LEVEL_ERROR || log_level == LEVEL_WARN ? stderr : stdout;

    fprintf(stream, "[%s] %s: ", level_strings[log_level], log_time());
    va_list args;
    va_start(args, msg);
    vfprintf(stream, msg, args);
    va_end(args);
    fprintf(stream, "\n");
}