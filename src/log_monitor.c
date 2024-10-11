#include "log_monitor.h"
#include "log_color.h"
#include "log_filter.h"
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

static int running = 1;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

long int critical_count = 0;
long int warning_count = 0;
long int info_count = 0;
long int debug_count = 0;

void handle_signal(int signal)
{
  if (signal == SIGINT)
  {
    running = 0;
    printf("\nExiting ...\n");
  }
}

// Function to compile regex patterns with case insensitivity
int compile_regex(regex_t *regex, const char *pattern)
{
  return regcomp(regex, pattern, REG_EXTENDED | REG_ICASE);
}

void *count_log_levels(void *arg)
{
  const char *line = (const char *)arg;

  regex_t regex;
  const char *patterns[] = {"\\|\\s*CRITICAL\\s*\\|", "\\|\\s*WARNING\\s*\\|",
                            "\\|\\s*INFO\\s*\\|", "\\|\\s*DEBUG\\s*\\|"};

  pthread_mutex_lock(&count_mutex);

  for (int i = 0; i < 4; i++)
  {
    if (compile_regex(&regex, patterns[i]) == 0 &&
        regexec(&regex, line, 0, NULL, 0) == 0)
    {
      switch (i)
      {
      case 0:
        critical_count++;
        break; // CRITICAL
      case 1:
        warning_count++;
        break; // WARNING
      case 2:
        info_count++;
        break; // INFO
      case 3:
        debug_count++;
        break; // DEBUG
      }
      break; // Exit after the first match
    }
    regfree(&regex);
  }

  pthread_mutex_unlock(&count_mutex);

  return NULL;
}

void process_lines(char *buffer, const char *filter_level, FILE *output_file)
{
  char *line = strtok(buffer, "\n");
  while (line != NULL)
  {
    if (should_print_log(line, filter_level))
    {
      colorize_log(line); // Apply colorization first

      // Output to file if specified
      if (output_file != NULL)
      {
        fprintf(output_file, "%s\n", line); // Write to file only
      }
      else
      {
        printf("%s\n", line); // Print to console only if no file is specified
      }

      count_log_levels(line); // Count log levels regardless of output destination
    }
    line = strtok(NULL, "\n");
  }
}

void print_statistics(FILE *output_file)
{
  if (output_file != NULL)
  {
    fprintf(output_file, "\nLog Statistics:\n");
    fprintf(output_file, "CRITICAL: %ld\n", critical_count);
    fprintf(output_file, "WARNING: %ld\n", warning_count);
    fprintf(output_file, "INFO: %ld\n", info_count);
    fprintf(output_file, "DEBUG: %ld\n", debug_count);
  }
  else
  {
    printf("\nLog Statistics:\n");
    printf("CRITICAL: %ld\n", critical_count);
    printf("WARNING: %ld\n", warning_count);
    printf("INFO: %ld\n", info_count);
    printf("DEBUG: %ld\n", debug_count);
  }
}

void start_log_monitor(const char *file_name, const char *filter_level,
                       int real_time, const char *output_file_name)
{
  signal(SIGINT, handle_signal);

  int fd = open(file_name, O_RDONLY);
  if (fd == -1)
  {
    perror("open");
    return;
  }

  char buffer[BUFFER_SIZE];
  FILE *output_file = NULL;

  if (output_file_name)
  {
    output_file = fopen(output_file_name, "w");
    if (!output_file)
    {
      perror("fopen");
      close(fd);
      return;
    }
  }

  if (!real_time)
  {
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
    {
      buffer[bytes_read] = '\0';
      process_lines(buffer, filter_level, output_file);
    }

    close(fd);

    print_statistics(output_file); // Pass output file for statistics

    if (output_file != NULL)
      fclose(output_file); // Close file if it was opened
    return;
  }

  // Real-time monitoring logic would go here...

  if (output_file != NULL)
    fclose(output_file); // Close file if it was opened
  close(fd);
}
