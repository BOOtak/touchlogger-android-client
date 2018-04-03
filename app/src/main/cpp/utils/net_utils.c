//
// Created by kirill on 12.11.17.
//

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#include "net_utils.h"
#include "../dirty/common/logging.h"

int init_connection(int port) {
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1)
  {
    LOGV("Unable to create TCP socket: %s!", strerror(errno));
    return -1;
  }

  LOGV("TCP Socket created!");

  struct sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  sockaddr.sin_port = htons(port);

  if (connect(sock_fd, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in)) == -1)
  {
    LOGV("Unable to connect to port :%d: %s!", port, strerror(errno));
    return -1;
  }

  LOGV("Connected to server!");
  return sock_fd;
}

ssize_t write_command(int sock_fd, const char *command) {
  return write(sock_fd, command, strlen(command));
}

int close_connection(int sock_fd) {
  return close(sock_fd);
}
