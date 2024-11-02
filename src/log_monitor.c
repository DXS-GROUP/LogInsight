#include "log_monitor.h"
#include "file_size.h"
#include "log_color.h"
#include "log_filter.h"
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

#define BUFFER_SIZE 524288 // Set to 512 KB
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * EVENT_SIZE) // Allocate space for 1024 events

#define RED "\033[0;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define WHITE "\033[1;37m"
#define ORANGE "\033[38;5;214m"
#define NC "\033[0m"

static int running = 1;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

long int critical_count = 0;
long int warning_count = 0;
long int info_count = 0;
long int debug_count = 0;
long int error_count = 0;
long int trace_count = 0;
long int unknown_count = 0;
long int fatal_count = 0;

void handle_signal(int signal) {
  if (signal == SIGINT) {
    running = 0;
    printf("\nExiting ...\n");
  }
}

int compile_regex(regex_t *regex, const char *pattern) {
  return regcomp(regex, pattern, REG_EXTENDED | REG_ICASE);
}

void *count_log_levels(void *arg) {
  const char *line = (const char *)arg;

  regex_t regex;
  const char *patterns[] = {"\\|\\s*CRITICAL\\s*\\|", "\\|\\s*WARNING\\s*\\|",
                            "\\|\\s*INFO\\s*\\|",     "\\|\\s*DEBUG\\s*\\|",
                            "\\|\\s*ERROR\\s*\\|",    "\\|\\s*UNKNOWN\\s*\\|",
                            "\\|\\s*TRACE\\s*\\|",    "\\|\\s*FATAL\\s*\\|"};

  pthread_mutex_lock(&count_mutex);

  int matched = 0; // Флаг для отслеживания совпадений

  for (int i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
    if (compile_regex(&regex, patterns[i]) == 0) {
      if (regexec(&regex, line, 0, NULL, 0) == 0) {
        matched = 1; // Установить флаг совпадения
        switch (i) {
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
        break; // Выход из цикла после первого совпадения
      }
      regfree(&regex); // Освобождение ресурсов регулярного выражения
    } else {
      fprintf(stderr, "Failed to compile regex: %s\n", patterns[i]);
    }
  }

  if (!matched) {
    unknown_count++; // Увеличиваем счетчик для неизвестных уровней
  }

  pthread_mutex_unlock(&count_mutex);

  return NULL;
}

void process_lines(char *buffer, const char *filter_level) {
  char *line = strtok(buffer, "\n");

  while (line != NULL) {
    if (should_print_log(line, filter_level)) {
      colorize_log(line);
      count_log_levels(line);
    }
    line = strtok(NULL, "\n");
  }
}

void print_statistics() {
  printf(BLUE "┌─────────────────────────────⬤ \n│    Log Statistics:\n");
  printf(BLUE "│" RED " ⬤ CRITICAL: %ld\n", critical_count);
  printf(BLUE "│" RED " ⬤ ERROR: %ld\n", error_count);
  printf(BLUE "│" ORANGE " ⬤ FATAL: %ld\n", fatal_count);
  printf(BLUE "│" YELLOW " ⬤ WARNING: %ld\n", warning_count);
  printf(BLUE "│" GREEN " ⬤ INFO: %ld\n", info_count);
  printf(BLUE "│" BLUE " ⬤ TRACE: %ld\n", trace_count);
  printf(BLUE "│"
              " ⬤ DEBUG: %ld\n" NC,
         debug_count);
  printf(BLUE "│" WHITE " ⬤ UNKNOWN: %ld\n", unknown_count);
  printf(BLUE "└─────────────────────────────⬤ \n");
}

void start_log_monitor(const char *file_name, const char *filter_level,
                       int real_time) {
  signal(SIGINT, handle_signal);

  start_monitoring();

  int fd = open(file_name, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return;
  }

  char buffer[BUFFER_SIZE];

  if (!real_time) {
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
      buffer[bytes_read] = '\0';
      process_lines(buffer, filter_level);
    }

    close(fd);
    print_file_size(file_name);
    print_statistics();
    stop_monitoring();
    return;
  }

  int inotify_fd = inotify_init();
  if (inotify_fd < 0) {
    perror("inotify_init");
    close(fd);
    return;
  }

  int wd = inotify_add_watch(inotify_fd, file_name, IN_MODIFY);
  if (wd == -1) {
    perror("inotify_add_watch");
    close(fd);
    close(inotify_fd);
    return;
  }

  off_t offset = lseek(fd, 0, SEEK_END);

  while (running) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(inotify_fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int retval = select(inotify_fd + 1, &readfds, NULL, NULL, &tv);
    if (retval == -1) {
      perror("select");
      break;
    } else if (retval == 0) {
      continue;
    }

    char event_buf[EVENT_BUF_LEN];
    ssize_t bytes = read(inotify_fd, event_buf, EVENT_BUF_LEN);

    if (bytes < 0) {
      perror("read");
      break;
    }

    lseek(fd, offset, SEEK_SET);
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
      buffer[bytes_read] = '\0';
      process_lines(buffer, filter_level);
      offset += bytes_read;
    }
  }

  inotify_rm_watch(inotify_fd, wd);

  print_file_size(file_name);
  print_statistics();

  stop_monitoring();
  close(fd);
}
