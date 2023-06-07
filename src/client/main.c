#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../common/includes.h"
#include "../common/util.h"

static int connection;

void *read_input(void *arg) {
    int res;
    char buffer[4096];

    while ((res = recv(connection, buffer, sizeof(buffer), 0)) > 0) {
        buffer[res] = '\0';
        printf("%s", buffer);
    }

    puts("keek");
    if (res == 0) { 
        close(connection);
        exit(EXIT_SUCCESS);
    }
    return NULL;
}

int main(int argc, char** argv) {
    char* host = argv[1];
    char* port = argv[2];
    
    /* Отправили серверу наши намерения */
    connection = create_connection(host, port);
    pack_argc(argc - 4, argv + 4, connection);

    /* Переходим в интерактивный режим */
    int res;
    char buffer[4096];
    while (1) {
        pthread_t id;
        pthread_create(&id, NULL, read_input, NULL);

        while ((res = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
            write(connection, &res, sizeof(res));
            write(connection, buffer, res);
        }
        int signal = Eof;
        write(connection, &signal, sizeof(signal));

        pthread_join(id, NULL);
    }
}