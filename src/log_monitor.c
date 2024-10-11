#include "log_monitor.h"
#include "log_filter.h"
#include "log_color.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 4096
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

static int running = 1; // Flag to control the main loop

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        running = 0; // Set flag to exit the loop
        printf("\nExiting gracefully...\n");
    }
}

void start_log_monitor(const char *file_name, const char *filter_level)
{
    signal(SIGINT, handle_signal);

    int fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return;
    }

    int inotify_fd = inotify_init();
    if (inotify_fd < 0)
    {
        perror("inotify_init");
        close(fd);
        return;
    }

    int wd = inotify_add_watch(inotify_fd, file_name, IN_MODIFY);
    if (wd == -1)
    {
        perror("inotify_add_watch");
        close(fd);
        close(inotify_fd);
        return;
    }

    off_t offset = lseek(fd, 0, SEEK_END); // Start reading from the end of the file
    char buffer[BUFFER_SIZE];

    // Initialize log statistics
    int critical_count = 0;
    int warning_count = 0;
    int info_count = 0;
    int debug_count = 0;

    while (running)
    {
        char event_buf[EVENT_BUF_LEN];
        int length = read(inotify_fd, event_buf, EVENT_BUF_LEN);
        if (length < 0)
        {
            perror("read");
            break;
        }

        lseek(fd, offset, SEEK_SET); // Move to the last known offset
        int bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == -1)
        {
            perror("read");
            break;
        }

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0'; // Null-terminate the string

            // Process each line in the buffer
            char *line = strtok(buffer, "\n");
            while (line != NULL)
            {
                if (should_print_log(line, filter_level))
                {
                    colorize_log(line); // Colorize and print the log line

                    // Update statistics based on log level
                    if (strstr(line, "| CRITICAL |"))
                        critical_count++;
                    else if (strstr(line, "| WARNING |"))
                        warning_count++;
                    else if (strstr(line, "| INFO |"))
                        info_count++;
                    else if (strstr(line, "| DEBUG |"))
                        debug_count++;
                }
                line = strtok(NULL, "\n");
            }
            offset += bytes_read; // Update the offset
        }
    }

    // Print statistics before exiting
    printf("\nLog Statistics:\n");
    printf("CRITICAL: %d\n", critical_count);
    printf("WARNING: %d\n", warning_count);
    printf("INFO: %d\n", info_count);
    printf("DEBUG: %d\n", debug_count);

    inotify_rm_watch(inotify_fd, wd);
    close(fd);
    close(inotify_fd);
}
