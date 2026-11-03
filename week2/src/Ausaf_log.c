#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256

static int read_frame(int pipe_fd, char *message)
{
    size_t received = 0;

    while (received < MAX_MSG_SIZE)
    {
        ssize_t bytes = read(
            pipe_fd,
            message + received,
            MAX_MSG_SIZE - received
        );

        if (bytes == 0)
        {
            if (received == 0)
            {
                return 0;
            }

            errno = EPIPE;
            return -1;
        }

        if (bytes == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return -1;
        }

        received += (size_t)bytes;
    }

    message[MAX_MSG_SIZE - 1] = '\0';
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "LOGGER: Usage: %s <pipe-read-fd>\n", argv[0]);
        return 1;
    }

    char *end;
    errno = 0;
    long pipe_fd_value = strtol(argv[1], &end, 10);
    if (
        errno != 0 ||
        end == argv[1] ||
        *end != '\0' ||
        pipe_fd_value < 0 ||
        pipe_fd_value > INT_MAX
    )
    {
        fprintf(stderr, "LOGGER: Invalid pipe descriptor\n");
        return 1;
    }
    int pipe_fd = (int)pipe_fd_value;

    FILE *log_file = fopen("simulator.log", "a");
    if (log_file == NULL)
    {
        perror("LOGGER: fopen");
        close(pipe_fd);
        return 1;
    }

    printf("\n");
    printf("====================================\n");
    printf("       susurkaddi LOGGER\n");
    printf("====================================\n");
    printf("LOGGER: Waiting for messages...\n");

    char message[MAX_MSG_SIZE];
    int read_status;
    while ((read_status = read_frame(pipe_fd, message)) == 1)
    {
        printf("LOG: %s\n", message);

        if (fprintf(log_file, "%s\n", message) < 0 || fflush(log_file) == EOF)
        {
            perror("LOGGER: writing log");
            fclose(log_file);
            close(pipe_fd);
            return 1;
        }

        if (strcmp(message, "Core shutting down") == 0)
        {
            break;
        }
    }

    if (read_status == -1)
    {
        perror("LOGGER: reading pipe");
        fclose(log_file);
        close(pipe_fd);
        return 1;
    }

    fclose(log_file);
    close(pipe_fd);

    printf("LOGGER: Shutdown complete.\n");
    return 0;
}
