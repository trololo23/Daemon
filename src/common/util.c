#include "util.h"
#include <stdio.h>
#include <string.h>
#include "includes.h"

int create_connection(char* node, char* service) {
    struct addrinfo* res = NULL;
    int gai_err;
    struct addrinfo hint = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};

    if ((gai_err = getaddrinfo(node, service, &hint, &res))) {
        return -1;
    }

    int sock = -1;
    for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            continue;
        }
        if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(res);
    return sock;
}

void pack_argc(int count, char** argv, int connection) {
    // now spawn only is available
    enum RequestType type = Spawn;
    write(connection, &type, sizeof(type));
    write(connection, &count, sizeof(count));

    for (size_t i = 0; i < count; ++i) {
        int size = strlen(argv[i]);
        write(connection, &size, sizeof(size));
        write(connection, argv[i], size);
    }
}

char** unpack(int connection) {
    char** argv;
    int args_count;

    int res = recv(connection, &args_count, sizeof(args_count), 0);
    argv = calloc(args_count, sizeof(*argv));

    int args_size;
    for (int i = 0; i < args_count; ++i) {
        int res = recv(connection, &args_size, sizeof(args_size), 0);
        argv[i] = calloc(args_size, sizeof(**argv));
        res = recv(connection, argv[i], args_size, 0);
    }
    return argv;
}

void free_argc(char** argv) {
    while (*argv) {
        free(*(argv++));
    }
}

void make_nonblocking_fd(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_listener(char* service) {
    struct addrinfo* res = NULL;
    int gai_err;
    struct addrinfo hint = {
        .ai_family = AF_UNSPEC,
        .ai_flags = AI_PASSIVE,
    };

    if ((gai_err = getaddrinfo(NULL, service, &hint, &res))) {
        fprintf(stderr, "%s\n", gai_strerror(gai_err));
        return -1;
    }

    int sock = -1;
    for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            continue;
        }

        if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(sock);
            sock = -1;
            continue;
        }

        if (listen(sock, SOMAXCONN) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);

    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    return sock;
}