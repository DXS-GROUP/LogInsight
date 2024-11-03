#include "log_statistics.h"

extern long int critical_count;
extern long int warning_count;
extern long int info_count;
extern long int error_count;
extern long int debug_count;
extern long int trace_count;
extern long int unknown_count;
extern long int fatal_count;

void print_statistics() {
  LogLevel log_levels[] = {
      {"CRITICAL", critical_count, "\033[1;31m"}, // RED
      {"ERROR", error_count, "\033[1;31m"},       // RED
      {"FATAL", fatal_count, "\033[38;5;214m"},   // ORANGE
      {"WARNING", warning_count, "\033[1;33m"},   // YELLOW
      {"INFO", info_count, "\033[1;32m"},         // GREEN
      {"TRACE", trace_count, "\033[1;34m"},       // BLUE
      {"DEBUG", debug_count, "\033[1;32m"},       // GREEN
      {"UNKNOWN", unknown_count, "\033[1;37m"}    // WHITE
  };

  printf("\033[1;34m┌─────────────────────────────⬤ \n│    Log Statistics:\n");

  for (int i = 0; i < sizeof(log_levels) / sizeof(log_levels[0]); i++) {
    if (log_levels[i].count > 0) {
      printf("\033[0;34m│%s ⬤ %s: %ld\n", log_levels[i].color,
             log_levels[i].label, log_levels[i].count);
    }
  }

  printf("\033[1;34m└─────────────────────────────⬤ \n");
}
