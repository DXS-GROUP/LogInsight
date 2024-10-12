#include "log_monitor.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char *program_name) {
  printf("\n");
  printf("\n\033[0;32m");
  printf("▄▄▌         ▄▄ • ▪   ▐ ▄ .▄▄ · ▪   ▄▄ •  ▄ .▄▄▄▄▄▄\n");
  printf("██•  ▪     ▐█ ▀ ▪██ •█▌▐█▐█ ▀. ██ ▐█ ▀ ▪██▪▐█•██  \n");
  printf("██▪   ▄█▀▄ ▄█ ▀█▄▐█·▐█▐▐▌▄▀▀▀█▄▐█·▄█ ▀█▄██▀▐█ ▐█.▪\n");
  printf("▐█▌▐▌▐█▌.▐▌▐█▄▪▐█▐█▌██▐█▌▐█▄▪▐█▐█▌▐█▄▪▐███▌▐▀ ▐█▌·\n");
  printf(".▀▀▀  ▀█▄▀▪·▀▀▀▀ ▀▀▀▀▀ █▪ ▀▀▀▀ ▀▀▀·▀▀▀▀ ▀▀▀ · ▀▀▀ \n");
  printf("\n");
  printf("\n\033[1;33m");
  printf("Usage: %s [-r] [-f <level>] -i <file> [-o <output>]\n", program_name);
  printf("  -r             Display all changes in real time\n");
  printf("  -f <level>     Level filtering (CRITICAL, WARNING, INFO, DEBUG)\n");
  printf("  -i <file>      Path to log file\n");
  printf("  -o <output>    Path to output file\n");
  printf("  -h, --help     Show this help\n\033[0m");
}

int main(int argc, char *argv[]) {
  const char *file_name = NULL;
  const char *filter_level = NULL;
  const char *output_file_name = NULL;
  int real_time = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      file_name = argv[++i];
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return EXIT_SUCCESS;
    } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      filter_level = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      output_file_name = argv[++i];
    } else if (strcmp(argv[i], "-r") == 0) {
      real_time = 1;
    }
  }

  if (!file_name) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  start_log_monitor(file_name, filter_level, real_time, output_file_name);

  return EXIT_SUCCESS;
}
