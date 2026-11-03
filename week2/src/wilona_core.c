// wilona

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define CORE_SOCKET_PATH "/tmp/susurkaddi_core.sock"
#define LOG_QUEUE      "/susurkaddi_log"

#define MAX_MSG_SIZE 256

#define MEMORY_SIZE 100
#define STACK_SIZE 100
#define QUEUE_SIZE 100


/* =========================
   MEMORY
   ========================= */

int memory[MEMORY_SIZE];


/* =========================
   STACK
   ========================= */

int stack[STACK_SIZE];
int stack_top = -1;

void push(int value)
{
    if (stack_top >= STACK_SIZE - 1)
    {
        printf("CORE: Stack overflow\n");
        return;
    }

    stack_top++;

    stack[stack_top] = value;

    printf("CORE: PUSH %d\n", value);
}

int pop(void)
{
    if (stack_top < 0)
    {
        printf("CORE: Stack underflow\n");
        return -1;
    }

    int value = stack[stack_top];

    stack_top--;

    printf("CORE: POP %d\n", value);

    return value;
}


/* =========================
   QUEUE
   ========================= */

int queue[QUEUE_SIZE];

int queue_front = 0;
int queue_rear = -1;

void enqueue(int value)
{
    if (queue_rear >= QUEUE_SIZE - 1)
    {
        printf("CORE: Queue full\n");
        return;
    }

    queue_rear++;

    queue[queue_rear] = value;

    printf("CORE: ENQUEUE %d\n", value);
}

int dequeue(void)
{
    if (queue_front > queue_rear)
    {
        printf("CORE: Queue empty\n");
        return -1;
    }

    int value = queue[queue_front];

    queue_front++;

    printf("CORE: DEQUEUE %d\n", value);

    return value;
}


/* =========================
   CPU
   ========================= */

int cpu_add(int a, int b)
{
    return a + b;
}

int cpu_sub(int a, int b)
{
    return a - b;
}

int cpu_mul(int a, int b)
{
    return a * b;
}

int cpu_div(int a, int b)
{
    if (b == 0)
    {
        return 0;
    }

    return a / b;
}


/* =========================
   SEND RESPONSE TO UI
   ========================= */

int send_frame(int socket_fd, const char *message)
{
    char frame[MAX_MSG_SIZE] = {0};
    size_t message_length = strlen(message);

    if (message_length >= sizeof(frame))
    {
        message_length = sizeof(frame) - 1;
    }

    memcpy(frame, message, message_length);

    size_t sent = 0;
    while (sent < sizeof(frame))
    {
        ssize_t bytes = send(
            socket_fd,
            frame + sent,
            sizeof(frame) - sent,
            0
        );

        if (bytes == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return -1;
        }

        if (bytes == 0)
        {
            errno = EPIPE;
            return -1;
        }

        sent += (size_t)bytes;
    }

    return 0;
}

int receive_frame(int socket_fd, char *buffer, size_t buffer_size)
{
    size_t received = 0;

    while (received < buffer_size)
    {
        ssize_t bytes = recv(
            socket_fd,
            buffer + received,
            buffer_size - received,
            0
        );

        if (bytes == 0)
        {
            if (received == 0)
            {
                return 0;
            }

            errno = ECONNRESET;
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

    buffer[buffer_size - 1] = '\0';
    return 1;
}

void send_response(int response_socket, const char *message)
{
    if (send_frame(response_socket, message) == -1)
    {
        perror("CORE: Failed to send response");
    }
}


/* =========================
   SEND LOG TO LOGGER
   ========================= */

void send_log(
    mqd_t log_queue,
    const char *message
)
{
    if (mq_send(
        log_queue,
        message,
        strlen(message) + 1,
        0
    ) == -1)
    {
        perror("CORE: Failed to send log");
    }
}


/* =========================
   MAIN
   ========================= */

int main(void)
{
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        perror("CORE: Cannot create socket");
        return 1;
    }

    if (
        unlink(CORE_SOCKET_PATH) == -1 &&
        errno != ENOENT
    )
    {
        perror("CORE: Cannot remove stale socket");
        close(server_socket);
        return 1;
    }

    struct sockaddr_un socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sun_family = AF_UNIX;
    strcpy(socket_address.sun_path, CORE_SOCKET_PATH);

    if (bind(
        server_socket,
        (struct sockaddr *)&socket_address,
        sizeof(socket_address)
    ) == -1)
    {
        perror("CORE: Cannot bind socket");
        close(server_socket);
        unlink(CORE_SOCKET_PATH);
        return 1;
    }

    if (listen(server_socket, 1) == -1)
    {
        perror("CORE: Cannot listen on socket");
        close(server_socket);
        unlink(CORE_SOCKET_PATH);
        return 1;
    }


    /* =========================
       CONNECT TO LOGGER
       ========================= */

    mqd_t log_queue = mq_open(
        LOG_QUEUE,
        O_WRONLY
    );

    if (log_queue == (mqd_t)-1)
    {
        perror("CORE: Cannot connect to logger");

        printf("CORE: Make sure LOGGER is running first.\n");

        close(server_socket);
        unlink(CORE_SOCKET_PATH);

        return 1;
    }

    signal(SIGPIPE, SIG_IGN);


    /* =========================
       START MESSAGE
       ========================= */

    printf("\n");
    printf("====================================\n");
    printf("         SusurKaddi CORE\n");
    printf("====================================\n");

    printf("CPU     : READY\n");
    printf("Memory  : READY\n");
    printf("Stack   : READY\n");
    printf("Queue   : READY\n");
    printf("Logger  : CONNECTED\n");

    printf("====================================\n");

    printf("CORE: Waiting for UI connection...\n");

    int ui_socket = accept(server_socket, NULL, NULL);

    if (ui_socket == -1)
    {
        perror("CORE: Failed to accept UI connection");
        mq_close(log_queue);
        close(server_socket);
        unlink(CORE_SOCKET_PATH);
        return 1;
    }

    printf("CORE: UI connected. Waiting for commands...\n");


    /* =========================
       MESSAGE BUFFER
       ========================= */

    char message[MAX_MSG_SIZE];


    /* =========================
       MAIN LOOP
       ========================= */

    while (1)
    {
        int frame_status = receive_frame(
            ui_socket,
            message,
            sizeof(message)
        );

        if (frame_status == -1)
        {
            perror("CORE: Failed to receive command");
            break;
        }

        if (frame_status == 0)
        {
            break;
        }

        printf("\nCORE received: %s\n", message);


        /* =========================
           EXIT
           ========================= */

        if (strcmp(message, "exit") == 0)
        {
            send_response(
                ui_socket,
                "Core shutting down"
            );

            send_log(
                log_queue,
                "Core shutting down"
            );

            break;
        }


        int a;
        int b;


        /* =========================
           ADD
           ========================= */

        if (sscanf(
            message,
            "ADD %d %d",
            &a,
            &b
        ) == 2)
        {
            int result = cpu_add(a, b);

            char response[MAX_MSG_SIZE];

            snprintf(
                response,
                sizeof(response),
                "ADD result = %d",
                result
            );

            send_response(
                ui_socket,
                response
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "ADD %d %d = %d",
                a,
                b,
                result
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        /* =========================
           SUB
           ========================= */

        if (sscanf(
            message,
            "SUB %d %d",
            &a,
            &b
        ) == 2)
        {
            int result = cpu_sub(a, b);

            char response[MAX_MSG_SIZE];

            snprintf(
                response,
                sizeof(response),
                "SUB result = %d",
                result
            );

            send_response(
                ui_socket,
                response
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "SUB %d %d = %d",
                a,
                b,
                result
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        /* =========================
           MUL
           ========================= */

        if (sscanf(
            message,
            "MUL %d %d",
            &a,
            &b
        ) == 2)
        {
            int result = cpu_mul(a, b);

            char response[MAX_MSG_SIZE];

            snprintf(
                response,
                sizeof(response),
                "MUL result = %d",
                result
            );

            send_response(
                ui_socket,
                response
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "MUL %d %d = %d",
                a,
                b,
                result
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        /* =========================
           DIV
           ========================= */

        if (sscanf(
            message,
            "DIV %d %d",
            &a,
            &b
        ) == 2)
        {
            if (b == 0)
            {
                send_response(
                    ui_socket,
                    "ERROR: Division by zero"
                );

                send_log(
                    log_queue,
                    "ERROR: Division by zero"
                );
            }
            else
            {
                int result = cpu_div(a, b);

                char response[MAX_MSG_SIZE];

                snprintf(
                    response,
                    sizeof(response),
                    "DIV result = %d",
                    result
                );

                send_response(
                    ui_socket,
                    response
                );


                char log_message[MAX_MSG_SIZE];

                snprintf(
                    log_message,
                    sizeof(log_message),
                    "DIV %d %d = %d",
                    a,
                    b,
                    result
                );

                send_log(
                    log_queue,
                    log_message
                );
            }

            continue;
        }


        int value;


        /* =========================
           PUSH
           ========================= */

        if (sscanf(
            message,
            "PUSH %d",
            &value
        ) == 1)
        {
            push(value);

            send_response(
                ui_socket,
                "Value pushed to stack"
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "PUSH %d",
                value
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        /* =========================
           POP
           ========================= */

        if (strcmp(message, "POP") == 0)
        {
            int result = pop();

            char response[MAX_MSG_SIZE];

            snprintf(
                response,
                sizeof(response),
                "POP result = %d",
                result
            );

            send_response(
                ui_socket,
                response
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "POP -> %d",
                result
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        /* =========================
           ENQUEUE
           ========================= */

        if (sscanf(
            message,
            "ENQUEUE %d",
            &value
        ) == 1)
        {
            enqueue(value);

            send_response(
                ui_socket,
                "Value added to queue"
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "ENQUEUE %d",
                value
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        /* =========================
           DEQUEUE
           ========================= */

        if (strcmp(message, "DEQUEUE") == 0)
        {
            int result = dequeue();

            char response[MAX_MSG_SIZE];

            snprintf(
                response,
                sizeof(response),
                "DEQUEUE result = %d",
                result
            );

            send_response(
                ui_socket,
                response
            );


            char log_message[MAX_MSG_SIZE];

            snprintf(
                log_message,
                sizeof(log_message),
                "DEQUEUE -> %d",
                result
            );

            send_log(
                log_queue,
                log_message
            );

            continue;
        }


        int address;


        /* =========================
           STORE
           ========================= */

        if (sscanf(
            message,
            "STORE %d %d",
            &address,
            &value
        ) == 2)
        {
            if (
                address >= 0 &&
                address < MEMORY_SIZE
            )
            {
                memory[address] = value;

                send_response(
                    ui_socket,
                    "Value stored in memory"
                );


                char log_message[MAX_MSG_SIZE];

                snprintf(
                    log_message,
                    sizeof(log_message),
                    "STORE Memory[%d] = %d",
                    address,
                    value
                );

                send_log(
                    log_queue,
                    log_message
                );
            }
            else
            {
                send_response(
                    ui_socket,
                    "ERROR: Invalid memory address"
                );

                send_log(
                    log_queue,
                    "ERROR: Invalid memory address"
                );
            }

            continue;
        }


        /* =========================
           LOAD
           ========================= */

        if (sscanf(
            message,
            "LOAD %d",
            &address
        ) == 1)
        {
            if (
                address >= 0 &&
                address < MEMORY_SIZE
            )
            {
                char response[MAX_MSG_SIZE];

                snprintf(
                    response,
                    sizeof(response),
                    "Memory[%d] = %d",
                    address,
                    memory[address]
                );

                send_response(
                    ui_socket,
                    response
                );


                char log_message[MAX_MSG_SIZE];

                snprintf(
                    log_message,
                    sizeof(log_message),
                    "LOAD Memory[%d] -> %d",
                    address,
                    memory[address]
                );

                send_log(
                    log_queue,
                    log_message
                );
            }
            else
            {
                send_response(
                    ui_socket,
                    "ERROR: Invalid memory address"
                );

                send_log(
                    log_queue,
                    "ERROR: Invalid memory address"
                );
            }

            continue;
        }


        /* =========================
           UNKNOWN COMMAND
           ========================= */

        send_response(
            ui_socket,
            "ERROR: Unknown command"
        );


        char log_message[MAX_MSG_SIZE];

        snprintf(
            log_message,
            sizeof(log_message),
            "ERROR: Unknown command -> %.220s",
            message
        );

        send_log(
            log_queue,
            log_message
        );
    }


    /* =========================
       CLEANUP
       ========================= */

    close(ui_socket);
    close(server_socket);
    mq_close(log_queue);

    unlink(CORE_SOCKET_PATH);

    printf("\nCORE: Shutdown complete.\n");

    return 0;
}
