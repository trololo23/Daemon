#pragma once

#include "includes.h"

int create_connection(char *node, char *service);

int create_listener(char* service);

void pack_argc(int count, char** argv, int connection);
char** unpack(int connection);

void free_argc(char** argv);

void make_nonblocking_fd(int fd);