#ifndef LOG_STATISTICS_H
#define LOG_STATISTICS_H

#include <stdio.h>

typedef struct {
  const char *label;
  long int count;
  const char *color;
} LogLevel;

void print_statistics();

#endif // LOG_STATISTICS_H
