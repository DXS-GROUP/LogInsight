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

static int running = 1; // Flag to control the main loop

// Mutex for protecting shared resources
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize log statistics
int critical_count = 0;
int warning_count = 0;
int info_count = 0;
int debug_count = 0;

void handle_signal(int signal) {
  if (signal == SIGINT) {
    running = 0; // Set flag to exit the loop
    printf("\nExiting gracefully...\n");
  }
}

// Function to compile regex patterns with case insensitivity
int compile_regex(regex_t *regex, const char *pattern) {
  return regcomp(regex, pattern, REG_EXTENDED | REG_ICASE);
}

void *count_log_levels(void *arg) {
  char *line = (char *)arg;

  regex_t regex;
  const char *patterns[] = {"\\|\\s*CRITICAL\\s*\\|", "\\|\\s*WARNING\\s*\\|",
                            "\\|\\s*INFO\\s*\\|", "\\|\\s*DEBUG\\s*\\|"};

  pthread_mutex_lock(&count_mutex); // Lock mutex before updating counts

  for (int i = 0; i < 4; i++) {
    if (compile_regex(&regex, patterns[i]) == 0 &&
        regexec(&regex, line, 0, NULL, 0) == 0) {
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
    }
    regfree(&regex);
  }

  pthread_mutex_unlock(&count_mutex); // Unlock mutex after updating counts

  return NULL;
}

void process_lines(char *buffer, const char *filter_level) {
  char *line = strtok(buffer, "\n");

  while (line != NULL) {
    if (should_print_log(line, filter_level)) {
      colorize_log(line); // Colorize and print the log line

      // Create a thread to count log levels
      pthread_t count_thread;
      pthread_create(&count_thread, NULL, count_log_levels, line);
      pthread_detach(
          count_thread); // Detach thread to allow it to run independently
    }
    line = strtok(NULL, "\n");
  }
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

  // If not in real-time mode, read and process existing log entries
  if (!real_time) {
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
      buffer[bytes_read] = '\0'; // Null-terminate the string

      process_lines(buffer, filter_level); // Process lines from buffer
    }

    close(fd); // Close file and exit after processing existing logs

    // Print statistics before exiting
    printf("\nLog Statistics:\n");
    printf("CRITICAL: %d\n", critical_count);
    printf("WARNING: %d\n", warning_count);
    printf("INFO: %d\n", info_count);
    printf("DEBUG: %d\n", debug_count);

    return;
  }

  // Initialize inotify for real-time monitoring only if -r is set
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

  off_t offset =
      lseek(fd, 0, SEEK_END); // Start reading from the end of the file

  while (running) {
    char event_buf[EVENT_BUF_LEN];
    int length = read(inotify_fd, event_buf, EVENT_BUF_LEN);

    if (length < 0) {
      perror("read");
      break;
    }

    lseek(fd, offset, SEEK_SET); // Move to the last known offset
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1) {
      perror("read");
      break;
    }

    if (bytes_read > 0) {
      buffer[bytes_read] = '\0'; // Null-terminate the string

      process_lines(buffer, filter_level); // Process lines from buffer

      offset += bytes_read; // Update the offset
    }
  }

  inotify_rm_watch(inotify_fd, wd);

  // Print statistics before exiting in real-time mode as well.
  printf("\nLog Statistics:\n");
  printf("CRITICAL: %d\n", critical_count);
  printf("WARNING: %d\n", warning_count);
  printf("INFO: %d\n", info_count);
  printf("DEBUG: %d\n", debug_count);

  close(fd);
  close(inotify_fd);
}