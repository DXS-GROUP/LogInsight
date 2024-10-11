#ifndef LOG_MONITOR_H
#define LOG_MONITOR_H

#include <pthread.h> // Include pthread for multithreading

// Function declarations
void start_log_monitor(const char *file_name, const char *filter_level,
                       int real_time);
void *count_log_levels(void *arg); // Function to count log levels

// Mutex for protecting shared resources
extern pthread_mutex_t count_mutex;

// Statistics variables
extern int critical_count;
extern int warning_count;
extern int info_count;
extern int debug_count;

#endif // LOG_MONITOR_H
