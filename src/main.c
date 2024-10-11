#include "log_monitor.h"
#include <stddef.h> // For NULL
#include <stdio.h>  // For fprintf and stderr
#include <stdlib.h> // For EXIT_FAILURE and EXIT_SUCCESS
#include <string.h> // For strcmp

int main(int argc, char *argv[]) {
  const char *file_name = NULL;
  const char *filter_level = NULL;
  int real_time = 0; // Flag for real-time output

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      file_name = argv[++i];
    } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      filter_level = argv[++i];
    } else if (strcmp(argv[i], "-r") == 0) // Check for -r flag
    {
      real_time = 1; // Set real-time flag
    }
  }

  if (!file_name) {
    fprintf(stderr, "Usage: %s -i <file> [-f <level>] [-r]\n", argv[0]);
    return EXIT_FAILURE;
  }

  start_log_monitor(file_name, filter_level,
                    real_time); // Pass real_time to the function

  return EXIT_SUCCESS;
}
