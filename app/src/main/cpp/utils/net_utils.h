//
// Created by kirill on 12.11.17.
//

#ifndef TOUCHLOGGER_DIRTY_NET_UTILS_H
#define TOUCHLOGGER_DIRTY_NET_UTILS_H

#ifdef __cplusplus
#define API_METHOD extern "C"
#else
#define API_METHOD
#endif

API_METHOD int init_connection(int port);

API_METHOD ssize_t write_command(int sock_fd, const char *command);

API_METHOD int close_connection(int sock_fd);

#endif //TOUCHLOGGER_DIRTY_NET_UTILS_H
