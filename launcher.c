#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <mqueue.h>

#define REQUEST_QUEUE  "/susurkaddi_request"
#define RESPONSE_QUEUE "/susurkaddi_response"
#define LOG_QUEUE      "/susurkaddi_log"

void cleanup_queues(void) {
    mq_unlink(REQUEST_QUEUE);
    mq_unlink(RESPONSE_QUEUE);
    mq_unlink(LOG_QUEUE);
}

int main(void) {
    printf("=================================================\n");
    printf("   SusurKaddi MULTI-PROCESS SIMULATOR LAUNCHER   \n");
    printf("=================================================\n");

    cleanup_queues();

    // 1. Launch Logger Process FIRST
    pid_t logger_pid = fork();
    if (logger_pid == 0) {
        execl("./logger", "./logger", NULL);
        perror("LAUNCHER: Failed to exec logger");
        exit(1);
    }
    sleep(1);

    // 2. Launch Core Process SECOND
    pid_t core_pid = fork();
    if (core_pid == 0) {
        execl("./core", "./core", NULL);
        perror("LAUNCHER: Failed to exec core");
        exit(1);
    }
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

    cleanup_queues();
    printf("\nLAUNCHER: All processes terminated successfully.\n");
    return 0;
}
