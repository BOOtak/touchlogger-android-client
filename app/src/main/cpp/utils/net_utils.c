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
#include <malloc.h>


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

ssize_t read_command(int sock_fd, char** command) {
  const int bufsize = 1024;
  char* buf = calloc(bufsize, sizeof(char));
  if (buf == NULL) {
    LOGV("Unable to allocate command buffer: %s!", strerror(errno));
    return -1;
  }

  *command = buf;
  char c;
  int index = 0;
  ssize_t readed;
  while ((readed = read(sock_fd, &c, 1)) == 1)
  {
    if (c == '\n')
    {
      return index;
    }

    buf[index++] = c;
  }

  if (readed == -1) {
    LOGV("Unable to read char from socket: %s!", strerror(errno));
  }

  return index;
}

int close_connection(int sock_fd) {
  return close(sock_fd);
}
