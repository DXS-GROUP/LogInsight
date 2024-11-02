// performance_monitor.h
#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>

void start_monitoring();
void stop_monitoring();

#endif // PERFORMANCE_MONITOR_H
