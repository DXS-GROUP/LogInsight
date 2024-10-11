#include "log_filter.h"
#include <string.h>
#include <regex.h>
#include <stdio.h>
#include <ctype.h>

int should_print_log(const char *line, const char *filter_level)
{
    if (filter_level == NULL || strlen(filter_level) == 0)
    {
        return 1; // No filtering applied
    }

    // Convert filter_level to lowercase for case-insensitive comparison
    char filter_level_lower[256];
    for (size_t i = 0; i < strlen(filter_level); i++)
    {
        filter_level_lower[i] = tolower((unsigned char)filter_level[i]);
    }
    filter_level_lower[strlen(filter_level)] = '\0'; // Null-terminate the string

    // Create a regex pattern to match the log level with optional spaces and case insensitivity
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\\|\\s*%s\\s*\\|", filter_level_lower);

    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE); // Use REG_ICASE for case-insensitive matching
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
