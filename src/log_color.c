#include "log_color.h"
#include <stdio.h>
#include <string.h>

// Define ANSI color codes
#define RED "\033[0;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define NC "\033[0m" // No Color

void colorize_log(const char *line)
{
    if (strstr(line, "| CRITICAL |") != NULL)
    {
        printf("%s%s%s\n", RED, line, NC);
    }
    else if (strstr(line, "| WARNING |") != NULL)
    {
        printf("%s%s%s\n", YELLOW, line, NC);
    }
    else if (strstr(line, "| INFO |") != NULL)
    {
        printf("%s%s%s\n", GREEN, line, NC);
    }
    else if (strstr(line, "| DEBUG |") != NULL)
    {
        printf("%s%s%s\n", BLUE, line, NC);
    }
    else
    {
        printf("%s\n", line); // Default output without coloring
    }
}
