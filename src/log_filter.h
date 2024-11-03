#ifndef LOG_FILTER_H
#define LOG_FILTER_H

#include <stddef.h>

int should_print_log(const char *line, char *filter_levels[], int filter_count);

#endif // LOG_FILTER_H
