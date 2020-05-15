//
// Created by kirill on 12.11.17.
//

#ifndef TOUCHLOGGER_DIRTY_NET_UTILS_H
#define TOUCHLOGGER_DIRTY_NET_UTILS_H

#include <api_method.h>

#define RESPONSE_OK "OK"
#define RESPONSE_ERROR "Error!"

API_METHOD int init_connection(int port);

API_METHOD ssize_t write_command(int sock_fd, const char* command);

API_METHOD ssize_t read_command(int sock_fd, char** command);

API_METHOD int close_connection(int sock_fd);

#endif //TOUCHLOGGER_DIRTY_NET_UTILS_H
