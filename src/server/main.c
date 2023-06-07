#include <stdlib.h>
#include <unistd.h>
#include "../common/includes.h"
#include "../common/util.h"
#include <wait.h>
#include <pthread.h>

void *wait_child(void *arg) {
    int pid = *(int*)arg;
    waitpid(pid, NULL, 0);
    exit(0);
}

void proxy_event_loop(int client, int pipe, int pid) {
    char buffer[4096];
    int res;
    while (1) {
        int header;
        if ((res = recv(client, &header, sizeof(header), 0)) > 0) {
            if (header == Eof) {
                close(pipe);
            } else if (header == Kill) {
                kill(pid, SIGKILL);
            }
            printf("%d\n", header);
            res = recv(client, buffer, header, 0);
            buffer[res] = '\0';
            write(pipe, buffer, res);
        } else {
            return;
        }
    }
}

void handle_spawn(int client) {
    char **argv = unpack(client);

    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();

    if (pid > 0) {
        close(pipefd[0]);

        pthread_t waiting;
        pthread_create(&waiting, NULL, wait_child, &pid);
        proxy_event_loop(client, pipefd[1], pid);

        close(client);
        close(pipefd[1]);
        pthread_kill(waiting, SIGKILL);
        pthread_join(waiting, NULL);
        exit(EXIT_SUCCESS);
    } else {
        fflush(stdout);
        dup2(pipefd[0], STDIN_FILENO);
        dup2(client, STDOUT_FILENO);
        close(client);
        close(pipefd[1]);
        execvp(argv[0], argv);
        exit(EXIT_FAILURE);
    }
}

int make_me_daemon() {
    pid_t pid = fork();
    if (pid) {
        exit(EXIT_SUCCESS);
    }
    setsid();
    return 0;
}

int main(int argc, char** argv) {
    char* port = argv[1];
    int sockfd = create_listener(argv[1]);
    make_me_daemon();

    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    while (1) {
        int connection = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
        pid_t pid = fork();
        if (!pid) {
            enum RequestType req_type;
            read(connection, &req_type, sizeof(req_type));

            if (req_type == Spawn) {
                handle_spawn(connection);
            } else {
                // coming soon
            }
            exit(EXIT_SUCCESS);
        } else {
            close(connection);
        }
    }
}