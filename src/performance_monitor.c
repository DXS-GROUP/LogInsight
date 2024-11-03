#include "performance_monitor.h"

static struct timeval start_time;
static struct rusage start_usage;

#define GREEN "\033[1;32m"

void start_monitoring() {
  gettimeofday(&start_time, NULL);
  getrusage(RUSAGE_SELF, &start_usage);
}

void stop_monitoring() {
  struct timeval end_time;
  struct rusage end_usage;

  gettimeofday(&end_time, NULL);
  getrusage(RUSAGE_SELF, &end_usage);

  long seconds = end_time.tv_sec - start_time.tv_sec;
  long microseconds = end_time.tv_usec - start_time.tv_usec;
  double elapsed = seconds + microseconds * 1e-6;

  long memory_used = end_usage.ru_maxrss;

  long user_cpu_time =
      end_usage.ru_utime.tv_sec * 1000000 + end_usage.ru_utime.tv_usec;
  long system_cpu_time =
      end_usage.ru_stime.tv_sec * 1000000 + end_usage.ru_stime.tv_usec;
  double cpu_time = (user_cpu_time + system_cpu_time) / 1000000.0;

  printf(GREEN "┌─────────────────────────────⬤ \n");
  if (elapsed < 1.0) {
    printf("│ ⬤  Elapsed time: %.3f milliseconds\n", elapsed * 1000);
  } else {
    printf("│ ⬤  Elapsed time: %.3f seconds\n", elapsed);
  }

  if (memory_used < 1024) {
    printf("│ ⬤  Memory used: %ld bytes\n", memory_used);
  } else if (memory_used < 1024 * 1024) {
    printf("│ ⬤  Memory used: %.2f KB\n", memory_used / 1024.0);
  } else {
    printf("│ ⬤  Memory used: %.2f MB\n", memory_used / (1024.0 * 1024.0));
  }

  if (cpu_time < 1.0) {
    printf("│ ⬤  CPU time: %.3f milliseconds\n", cpu_time * 1000);
  } else {
    printf("│ ⬤  CPU time: %.3f seconds\n", cpu_time);
  }
  printf("└─────────────────────────────⬤ \n");
}
