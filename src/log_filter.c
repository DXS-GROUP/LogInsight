#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>

#define MAX_PATTERN_LENGTH 256
#define MAX_FILTER_LENGTH 256

int should_print_log(const char *line, char *filter_levels[],
                     int filter_count) {
  if (filter_count == 0) {
    return 1;
  }

  for (int j = 0; j < filter_count; j++) {
    char filter_level_lower[MAX_FILTER_LENGTH];
    size_t filter_length = strlen(filter_levels[j]);

    for (size_t i = 0; i < filter_length; i++) {
      filter_level_lower[i] = tolower((unsigned char)filter_levels[j][i]);
    }
    filter_level_lower[filter_length] = '\0';

    char pattern[MAX_PATTERN_LENGTH];

    int written = snprintf(pattern, sizeof(pattern), "\\|\\s*%s\\s*\\|",
                           filter_level_lower);

    if (written < 0 || written >= sizeof(pattern)) {
      fprintf(stderr, "Error formatting regex pattern.\n");
      return 0;
    }

    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);

    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return 0;
    }

    reti = regexec(&regex, line, 0, NULL, 0);

    regfree(&regex);

    if (reti == 0) {
      return 1;
    }
  }

  return 0;
}
