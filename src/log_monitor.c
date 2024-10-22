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
#include <sys/select.h>
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
                            "\\|\\s*INFO\\s*\\|", "\\|\\s*DEBUG\\s*\\|"};

  pthread_mutex_lock(&count_mutex);

  for (int i = 0; i < 4; i++) {
    if (compile_regex(&regex, patterns[i]) == 0) {
      if (regexec(&regex, line, 0, NULL, 0) == 0) {
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
        }
        break; // Exit after the first match
      }
      regfree(&regex); // Free regex resources here
    } else {
      fprintf(stderr, "Failed to compile regex: %s\n", patterns[i]);
    }
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
  printf("\nLog Statistics:\n");
  printf("CRITICAL: %ld\n", critical_count);
  printf("WARNING: %ld\n", warning_count);
  printf("INFO: %ld\n", info_count);
  printf("DEBUG: %ld\n", debug_count);
}

void start_log_monitor(const char *file_name, const char *filter_level,
                       int real_time) {
  signal(SIGINT, handle_signal);

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
    print_statistics();
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

  print_statistics();

  close(fd);
}
