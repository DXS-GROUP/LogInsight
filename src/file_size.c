#include <stdio.h>
#include <sys/stat.h>

#define RED "\033[0;31m"
#define NC "\033[0m"

void print_file_size(const char *file_name) {
  struct stat st;
  if (stat(file_name, &st) == 0) {
    long long size = st.st_size;
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;

    while (size >= 1024 && unit_index < sizeof(units) / sizeof(units[0]) - 1) {
      size /= 1024;
      unit_index++;
    }

    printf(RED "\n┌─────────────────────────────\n│ ⚪ File size: %.2f "
               "%s\n└─────────────────────────────\n" NC,
           (double)size, units[unit_index]);
  } else {
    perror(RED "\n\nCould not get file size" NC);
  }
}
