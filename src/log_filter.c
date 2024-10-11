#include "log_filter.h"
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>

int should_print_log(const char *line, const char *filter_level) {
  if (filter_level == NULL || strlen(filter_level) == 0) {
    return 1;
  }

  char filter_level_lower[256];
  for (size_t i = 0; i < strlen(filter_level); i++) {
    filter_level_lower[i] = tolower((unsigned char)filter_level[i]);
  }
  filter_level_lower[strlen(filter_level)] = '\0';

  char pattern[256];
  snprintf(pattern, sizeof(pattern), "\\|\\s*%s\\s*\\|", filter_level_lower);

  regex_t regex;
  int reti = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    return 0;
  }

  reti = regexec(&regex, line, 0, NULL, 0);
  regfree(&regex);

  return reti == 0;
}
