#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "config.h"

volatile sig_atomic_t stop_server = 0;

void signal_handler(int sig) {
    stop_server = 1;
}

char *get_full_file_path(const char *filename) {
    static char full_path[1024];
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return NULL;
    }

    snprintf(full_path, sizeof(full_path), "%s/%s", cwd, filename);

    printf("Full path to file: %s\n", full_path);

    if (access(full_path, F_OK) == -1) {
        perror("File does not exist");
        return NULL;
    }

    return full_path;
}

void *handle_client(void *arg) {
    int new_socket = *(int *)arg;
    char buffer[1024];
    ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(new_socket);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    printf("Received request:\n%s\n", buffer);

    char *path_start = buffer + 4;
    char *path_end = strchr(path_start, ' ');
    if (path_end) {
        *path_end = '\0';
    }

    char *filename = NULL;

    for (int i = 0; ROUTES[i].route != NULL; i++) {
        if (strcmp(path_start, ROUTES[i].route) == 0) {
            filename = ROUTES[i].filename;
            break;
        }
    }

    printf("Serving file: %s\n", filename ? filename : "not found");

    if (filename == NULL) {
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        write(new_socket, response, strlen(response));
    } else {
        char *full_path = get_full_file_path(filename);
        if (full_path == NULL) {
            char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
            write(new_socket, response, strlen(response));
            close(new_socket);
            return NULL;
        }

        int fd = open(full_path, O_RDONLY);
        if (fd == -1) {
            perror("open");
            close(new_socket);
            return NULL;
        }

        char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        write(new_socket, response_header, strlen(response_header));

        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            write(new_socket, buffer, bytes_read);
        }

        close(fd);
    }

    close(new_socket);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    signal(SIGINT, signal_handler);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d. Press Ctrl+C to stop.\n", PORT);

    while (!stop_server) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (errno == EINTR) continue;
            perror("accept failed");
            break;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&new_socket) < 0) {
            perror("pthread_create failed");
            close(new_socket);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(server_fd);
    printf("Server stopped.\n");
    return 0;
}
