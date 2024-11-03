#ifndef LOG_MONITOR_H
#define LOG_MONITOR_H

#include <pthread.h>

void start_log_monitor(const char *file_name, char *filter_levels[],
                       int filter_count, int real_time, int show_stats);
void process_line(const char *line, char *filter_levels[], int filter_count);
void count_log_levels(const char *line);

extern pthread_mutex_t count_mutex;

extern long int critical_count;
extern long int warning_count;
extern long int info_count;
extern long int error_count;
extern long int debug_count;
extern long int trace_count;
extern long int unknown_count;
extern long int fatal_count;

#endif // LOG_MONITOR_H
