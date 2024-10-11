#ifndef LOG_MONITOR_H
#define LOG_MONITOR_H

#include <pthread.h> // Include pthread for multithreading

void start_log_monitor(const char *file_name, const char *filter_level);
void *count_log_levels(void *arg); // Function to count log levels

// Mutex for protecting shared resources
extern pthread_mutex_t count_mutex;

#endif // LOG_MONITOR_H
