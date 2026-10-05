#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 100
#define STACK_SIZE 100
#define QUEUE_SIZE 100

int memory[MEMORY_SIZE];
int stack[STACK_SIZE];
int stack_top = -1;
int queue[QUEUE_SIZE];
int queue_front = 0, queue_rear = -1;

void process_command(const char *cmd, FILE *log_file) {
    int a, b, val, addr;
    if (sscanf(cmd, "ADD %d %d", &a, &b) == 2) {
        int res = a + b;
        fprintf(log_file, "ADD %d %d = %d\n", a, b, res);
    } else if (sscanf(cmd, "STORE %d %d", &addr, &val) == 2) {
        if (addr >= 0 && addr < MEMORY_SIZE) memory[addr] = val;
        fprintf(log_file, "STORE Memory[%d] = %d\n", addr, val);
    } else if (sscanf(cmd, "PUSH %d", &val) == 1) {
        if (stack_top < STACK_SIZE - 1) stack[++stack_top] = val;
        fprintf(log_file, "PUSH %d\n", val);
    } else if (strcmp(cmd, "POP") == 0) {
        if (stack_top >= 0) {
            val = stack[stack_top--];
            fprintf(log_file, "POP -> %d\n", val);
        }
    } else if (sscanf(cmd, "ENQUEUE %d", &val) == 1) {
        if (queue_rear < QUEUE_SIZE - 1) queue[++queue_rear] = val;
        fprintf(log_file, "ENQUEUE %d\n", val);
    } else if (strcmp(cmd, "DEQUEUE") == 0) {
        if (queue_front <= queue_rear) {
            val = queue[queue_front++];
            fprintf(log_file, "DEQUEUE -> %d\n", val);
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }

    FILE *log_file = fopen("standalone.log", "w");
    if (!log_file) {
        perror("Failed to open standalone.log");
        return 1;
    }

    for (int i = 0; i < iterations; i++) {
        process_command("STORE 10 42", log_file);
        process_command("ADD 10 20", log_file);
        process_command("PUSH 99", log_file);
        process_command("POP", log_file);
        process_command("ENQUEUE 7", log_file);
        process_command("DEQUEUE", log_file);
    }

    fclose(log_file);
    printf("Standalone simulation completed %d iterations.\n", iterations);
    return 0;
}
