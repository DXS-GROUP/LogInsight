#include "log_color.h"
#include <regex.h>
#include <stdio.h>
#include <string.h>

#define RED "\033[0;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define NC "\033[0m"

void colorize_log(const char *line) {
  regex_t regex;
  int reti;

  const char *patterns[] = {"\\|\\s*CRITICAL\\s*\\|", "\\|\\s*WARNING\\s*\\|",
                            "\\|\\s*INFO\\s*\\|", "\\|\\s*DEBUG\\s*\\|",
                            "\\|\\s*ERROR\\s*\\|"};

  for (int i = 0; i < 5; i++) {
    reti = regcomp(&regex, patterns[i], REG_EXTENDED | REG_ICASE);
    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return;
    }

    reti = regexec(&regex, line, 0, NULL, 0);
    if (reti == 0) {
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
      case 4: // ERROR
        printf("%s%s%s\n", RED, line, NC);
        break;
      }
      regfree(&regex);
      return;
    }
    regfree(&regex);
  }

  printf("%s\n", line);
}
