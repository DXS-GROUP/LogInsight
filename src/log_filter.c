#include "log_filter.h"
#include <string.h>
#include <regex.h>
#include <stdio.h>

int should_print_log(const char *line, const char *filter_level)
{
    if (filter_level == NULL || strlen(filter_level) == 0)
    {
        return 1; // No filtering applied
    }

    // Create a regex pattern to match the log level with optional spaces
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\\|\\s*%s\\s*\\|", filter_level);

    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti)
    {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    // Execute regex
    reti = regexec(&regex, line, 0, NULL, 0);
    regfree(&regex); // Free memory allocated to the pattern

    return reti == 0; // Return true if the regex matched
}
