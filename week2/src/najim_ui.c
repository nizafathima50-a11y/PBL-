#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define CORE_SOCKET_PATH "/tmp/susurkaddi_core.sock"

#define MAX_MSG_SIZE 256

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

int main(void)
{
    signal(SIGPIPE, SIG_IGN);

    int core_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (core_socket == -1)
    {
        perror("UI: Cannot create socket");
        return 1;
    }

    struct sockaddr_un socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sun_family = AF_UNIX;
    strcpy(socket_address.sun_path, CORE_SOCKET_PATH);

    if (connect(
        core_socket,
        (struct sockaddr *)&socket_address,
        sizeof(socket_address)
    ) == -1)
    {
        perror("UI: Cannot connect to Core");
        printf("UI: Make sure CORE is running first.\n");
        close(core_socket);
        return 1;
    }


    /* =========================
       UI HEADER
       ========================= */

    printf("\n");
    printf("====================================\n");
    printf("        susurkaddi UI\n");
    printf("====================================\n");

    printf("Commands:\n");

    printf("ADD a b\n");
    printf("SUB a b\n");
    printf("MUL a b\n");
    printf("DIV a b\n");

    printf("PUSH value\n");
    printf("POP\n");

    printf("ENQUEUE value\n");
    printf("DEQUEUE\n");

    printf("STORE address value\n");
    printf("LOAD address\n");

    printf("exit\n");

    printf("====================================\n");


    char message[MAX_MSG_SIZE];
    char response[MAX_MSG_SIZE];


    /* =========================
       UI LOOP
       ========================= */

    while (1)
    {
        printf("\nUI > ");

        fflush(stdout);


        if (fgets(
            message,
            sizeof(message),
            stdin
        ) == NULL)
        {
            break;
        }


        /* Remove newline */

        message[
            strcspn(message, "\n")
        ] = '\0';


        /* Ignore empty input */

        if (strlen(message) == 0)
        {
            continue;
        }


        /* =========================
           SEND COMMAND TO CORE
           ========================= */

        if (send_frame(core_socket, message) == -1)
        {
            perror("UI: Failed to send command");
            break;
        }


        /* =========================
           WAIT FOR CORE RESPONSE
           ========================= */

        int frame_status = receive_frame(
            core_socket,
            response,
            sizeof(response)
        );

        if (frame_status == -1)
        {
            perror("UI: Failed to receive response");
            break;
        }

        if (frame_status == 0)
        {
            printf("UI: Core disconnected.\n");
            break;
        }


        printf("CORE: %s\n", response);


        /* =========================
           EXIT
           ========================= */

        if (strcmp(message, "exit") == 0)
        {
            break;
        }
    }


    /* =========================
       CLEANUP
       ========================= */

    close(core_socket);

    printf("\nUI: Shutdown complete.\n");

    return 0;
}
