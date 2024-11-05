#include "log_monitor.h"
#include "file_size.h"
#include "log_color.h"
#include "log_filter.h"
#include "log_statistics.h"
#include "performance_monitor.h"
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define INITIAL_BUFFER_SIZE 1024 // Начальный размер буфера для чтения
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * EVENT_SIZE)

#define RED "\033[0;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define WHITE "\033[1;37m"
#define ORANGE "\033[38;5;214m"
#define NC "\033[0m"

long int critical_count = 0;
long int warning_count = 0;
long int info_count = 0;
long int debug_count = 0;
long int error_count = 0;
long int trace_count = 0;
long int unknown_count = 0;
long int fatal_count = 0;

regex_t regex_patterns[8];
static int running = 1;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_signal(int signal)
{
  if (signal == SIGINT)
  {
    running = 0;
    printf(RED "\n\nBye...\n");
  }
}

void compile_regex_patterns()
{
  const char *patterns[] = {
      "\\|\\s*CRITICAL\\s*\\|", "\\|\\s*WARNING\\s*\\|",
      "\\|\\s*INFO\\s*\\|", "\\|\\s*DEBUG\\s*\\|",
      "\\|\\s*ERROR\\s*\\|", "\\|\\s*UNKNOWN\\s*\\|",
      "\\|\\s*TRACE\\s*\\|", "\\|\\s*FATAL\\s*\\|"};

  for (int i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++)
  {
    if (regcomp(&regex_patterns[i], patterns[i], REG_EXTENDED | REG_ICASE) != 0)
    {
      fprintf(stderr, "Failed to compile regex: %s\n", patterns[i]);
      exit(EXIT_FAILURE);
    }
  }
}

void free_regex_patterns()
{
  for (int i = 0; i < sizeof(regex_patterns) / sizeof(regex_patterns[0]); i++)
  {
    regfree(&regex_patterns[i]);
  }
}

void count_log_levels(const char *line)
{
  pthread_mutex_lock(&count_mutex);

  int matched = 0;

  for (int i = 0; i < sizeof(regex_patterns) / sizeof(regex_patterns[0]); i++)
  {
    if (regexec(&regex_patterns[i], line, 0, NULL, 0) == 0)
    {
      matched = 1;
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
      case 4:
        error_count++;
        break; // ERROR
      case 5:
        unknown_count++;
        break; // UNKNOWN
      case 6:
        trace_count++;
        break; // TRACE
      case 7:
        fatal_count++;
        break; // FATAL
      }
      break;
    }
  }

  if (!matched)
  {
    unknown_count++;
  }

  pthread_mutex_unlock(&count_mutex);
}

void process_line(const char *line, char *filter_levels[], int filter_count)
{
  if (should_print_log(line, filter_levels, filter_count))
  {
    colorize_log(line);
    count_log_levels(line);
  }
}

void start_log_monitor(const char *file_name, char *filter_levels[], int filter_count, int real_time, int show_stats)
{
  signal(SIGINT, handle_signal);

  compile_regex_patterns();
  if (show_stats)
  {
    start_monitoring();
  }

  int fd = open(file_name, O_RDONLY);
  if (fd == -1)
  {
    perror("open");
    free_regex_patterns();
    return;
  }

  char *buffer = malloc(INITIAL_BUFFER_SIZE);
  size_t buffer_size = INITIAL_BUFFER_SIZE;
  size_t current_length = 0;

  if (!real_time)
  {
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer + current_length, buffer_size - current_length - 1)) > 0)
    {
      current_length += bytes_read;
      buffer[current_length] = '\0';

      char *line_start = buffer;
      char *line_end;

      while ((line_end = strchr(line_start, '\n')) != NULL)
      {
        *line_end = '\0';
        process_line(line_start, filter_levels, filter_count);
        line_start = line_end + 1;
      }

      // Move remaining data to the beginning of the buffer
      current_length -= (line_start - buffer);
      memmove(buffer, line_start, current_length);
    }

    close(fd);
    print_file_size(file_name);
    print_statistics();
    if (show_stats)
    {
      stop_monitoring();
    }
    free_regex_patterns();
    free(buffer);
    return;
  }

  int inotify_fd = inotify_init();
  if (inotify_fd < 0)
  {
    perror("inotify_init");
    close(fd);
    free(buffer);
    return;
  }

  int wd = inotify_add_watch(inotify_fd, file_name, IN_MODIFY);
  if (wd == -1)
  {
    perror("inotify_add_watch");
    close(fd);
    close(inotify_fd);
    free(buffer);
    return;
  }

  off_t offset = lseek(fd, 0, SEEK_END);

  while (running)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(inotify_fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int retval = select(inotify_fd + 1, &readfds, NULL, NULL, &tv);

    if (retval == -1)
    {
      perror("select");
      break;
    }
    else if (retval == 0)
    {
      continue;
    }

    char event_buf[EVENT_BUF_LEN];

    ssize_t bytes = read(inotify_fd, event_buf, EVENT_BUF_LEN);

    if (bytes < 0)
    {
      perror("read");
      break;
    }

    lseek(fd, offset, SEEK_SET);
    ssize_t bytes_read = read(fd, buffer + current_length, buffer_size - current_length - 1);

    if (bytes_read > 0)
    {
      current_length += bytes_read;
      buffer[current_length] = '\0';

      char *line_start = buffer;
      char *line_end;

      while ((line_end = strchr(line_start, '\n')) != NULL)
      {
        *line_end = '\0';
        process_line(line_start, filter_levels, filter_count);
        line_start = line_end + 1;
      }

      // Move remaining data to the beginning of the buffer
      current_length -= (line_start - buffer);
      memmove(buffer, line_start, current_length);
      offset += bytes_read;
    }
  }

  inotify_rm_watch(inotify_fd, wd);

  print_file_size(file_name);
  print_statistics();
  free_regex_patterns();
  stop_monitoring();

  close(fd);
  free(buffer);
}
