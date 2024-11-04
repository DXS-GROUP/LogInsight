#include "performance_monitor.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

static struct timeval start_time;
static struct rusage start_usage;

#define GREEN "\033[1;32m"

void start_monitoring()
{
  gettimeofday(&start_time, NULL);
  getrusage(RUSAGE_SELF, &start_usage);
}

void stop_monitoring()
{
  struct timeval end_time;
  struct rusage end_usage;

  gettimeofday(&end_time, NULL);
  getrusage(RUSAGE_SELF, &end_usage);

  double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_usec - start_time.tv_usec) * 1e-6;

  long memory_used = end_usage.ru_maxrss;
  double cpu_time = (end_usage.ru_utime.tv_sec + end_usage.ru_stime.tv_sec) +
                    (end_usage.ru_utime.tv_usec + end_usage.ru_stime.tv_usec) * 1e-6;

  printf(GREEN "┌─────────────────────────────⬤ \n");
  printf("│ ⬤  Elapsed time: %.3f %s\n",
         elapsed < 1.0 ? elapsed * 1000 : elapsed,
         elapsed < 1.0 ? "milliseconds" : "seconds");

  if (memory_used < 1024)
  {
    printf("│ ⬤  Memory used: %ld bytes\n", memory_used);
  }
  else if (memory_used < 1024 * 1024)
  {
    printf("│ ⬤  Memory used: %.2f KB\n", memory_used / 1024.0);
  }
  else
  {
    printf("│ ⬤  Memory used: %.2f MB\n", memory_used / (1024.0 * 1024.0));
  }

  printf("│ ⬤  CPU time: %.3f %s\n",
         cpu_time < 1.0 ? cpu_time * 1000 : cpu_time,
         cpu_time < 1.0 ? "milliseconds" : "seconds");

  printf("└─────────────────────────────⬤ \n");
}
