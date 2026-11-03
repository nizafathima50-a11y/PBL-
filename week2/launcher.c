#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    printf("=================================================\n");
    printf("   SusurKaddi MULTI-PROCESS SIMULATOR LAUNCHER   \n");
    printf("=================================================\n");

    int log_pipe[2];
    if (pipe(log_pipe) == -1) {
        perror("LAUNCHER: Cannot create log pipe");
        return 1;
    }

    // 1. Launch Logger Process FIRST
    pid_t logger_pid = fork();
    if (logger_pid == -1) {
        perror("LAUNCHER: Cannot fork logger");
        close(log_pipe[0]);
        close(log_pipe[1]);
        return 1;
    }
    if (logger_pid == 0) {
        close(log_pipe[1]);
        char read_fd[16];
        snprintf(read_fd, sizeof(read_fd), "%d", log_pipe[0]);
        execl("./logger", "./logger", read_fd, NULL);
        perror("LAUNCHER: Failed to exec logger");
        exit(1);
    }

    // 2. Launch Core Process SECOND
    pid_t core_pid = fork();
    if (core_pid == -1) {
        perror("LAUNCHER: Cannot fork core");
        close(log_pipe[0]);
        close(log_pipe[1]);
        waitpid(logger_pid, NULL, 0);
        return 1;
    }
    if (core_pid == 0) {
        close(log_pipe[0]);
        char write_fd[16];
        snprintf(write_fd, sizeof(write_fd), "%d", log_pipe[1]);
        execl("./core", "./core", write_fd, NULL);
        perror("LAUNCHER: Failed to exec core");
        exit(1);
    }
    close(log_pipe[0]);
    close(log_pipe[1]);
    sleep(1);

    // 3. Launch UI Process THIRD
    pid_t ui_pid = fork();
    if (ui_pid == 0) {
        execl("./ui", "./ui", NULL);
        perror("LAUNCHER: Failed to exec ui");
        exit(1);
    }

    waitpid(ui_pid, NULL, 0);
    waitpid(core_pid, NULL, 0);
    waitpid(logger_pid, NULL, 0);

    printf("\nLAUNCHER: All processes terminated successfully.\n");
    return 0;
}
