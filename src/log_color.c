#include "log_color.h"
#include <regex.h>
#include <stdio.h>
#include <string.h>

// Define ANSI color codes
#define RED "\033[0;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define NC "\033[0m" // No Color

void colorize_log(const char *line) {
  regex_t regex;
  int reti;

  // Define patterns for each log level with optional spaces and case
  // insensitivity
  const char *patterns[] = {
      "\\|\\s*CRITICAL\\s*\\|", // Pattern for CRITICAL
      "\\|\\s*WARNING\\s*\\|",  // Pattern for WARNING
      "\\|\\s*INFO\\s*\\|",     // Pattern for INFO
      "\\|\\s*DEBUG\\s*\\|"     // Pattern for DEBUG
  };

  // Check each pattern against the log line
  for (int i = 0; i < 4; i++) {
    reti =
        regcomp(&regex, patterns[i],
                REG_EXTENDED |
                    REG_ICASE); // Use REG_ICASE for case-insensitive matching
    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return;
    }

    // Execute regex
    reti = regexec(&regex, line, 0, NULL, 0);
    if (reti == 0) { // If a match is found
      switch (i) {
      case 0: // CRITICAL
        printf("%s%s%s\n", RED, line, NC);
        break;
      case 1: // WARNING
        printf("%s%s%s\n", YELLOW, line, NC);
        break;
      case 2: // INFO
        printf("%s%s%s\n", GREEN, line, NC);
        break;
      case 3: // DEBUG
        printf("%s%s%s\n", BLUE, line, NC);
        break;
      }
      regfree(&regex); // Free regex memory after use
      return;          // Exit after printing the colored log line
    }
    regfree(&regex); // Free regex memory after use
  }

  printf("%s\n", line); // Default output without coloring if no match is found
}
