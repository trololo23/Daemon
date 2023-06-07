#pragma once

#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

enum { MAX_REQUEST_SIZE = 512 };

enum RequestType {
    Spawn = -2,
    List,
    Attach,
    Kill = -1,
    Eof = 0
};