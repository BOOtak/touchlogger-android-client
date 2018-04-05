//
// Created by kirill on 14.11.17.
//

#include <sys/socket.h>
#include <linux/in.h>
#include <endian.h>
#include <unistd.h>
#include <string.h>
#include "ControlReader.h"
#include "dirty/common/logging.h"

ControlReader::ControlReader(int port,
                             std::map<std::string, control_callback> &commands)
    : port(port), commands(commands), sockFd(-1), shouldStop(0)
{}

void ControlReader::start()
{
  startServerThread();
}

int ControlReader::startServerThread()
{
  pthread_t server_thread = -1;
  int res = pthread_create(&server_thread, NULL, &serverRoutine, (void*) this);
  if (res == -1)
  {
    LOGV("Unable to start server: %s!", strerror(errno));
    return -1;
  }

  LOGV("Got server started.");
  return 0;
}

void* ControlReader::serverRoutine(void* instance)
{
  ControlReader* cls = reinterpret_cast<ControlReader*>(instance);

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1)
  {
    LOGV("Unable to create TCP socket: %s!", strerror(errno));
    return NULL;
  }

  struct sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(cls->port);

  if (bind(sock_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) == -1)
  {
    LOGV("Unable to bind on port :%d: %s!", cls->port, strerror(errno));
    return NULL;
  }

  printf("Listening for connections...\n");
  if (listen(sock_fd, 1) == -1)
  {
    LOGV("Unable to listen: %s!", strerror(errno));
    close(sock_fd);
    return NULL;
  }

  LOGV("Waiting for connection...");
  struct sockaddr_in client_addr;
  socklen_t size = sizeof(client_addr);

  while (__sync_bool_compare_and_swap(&(cls->shouldStop), 0, 0))
  {
    int acceptedConnection = accept(sock_fd, (struct sockaddr*) &client_addr, &size);
    if (acceptedConnection == -1)
    {
      LOGV("Unable to accept connection: %s!", strerror(errno));
      close(sock_fd);
      return NULL;
    }

    if (cls->connectionRoutine(acceptedConnection) == -1)
    {
      LOGV("Connection handled ended abnormally!");
    }

    close(acceptedConnection);
  }

  return NULL;
}

int ControlReader::connectionRoutine(int clientFd)
{
  size_t commandLength = 1024;
  char commandBuffer[commandLength], c;
  ssize_t status;
  int readed = 0;

  const char* responseOk = "OK\n";
  const char* responseError = "Error!\n";

  while (__sync_bool_compare_and_swap(&shouldStop, 0, 0))
  {
    if ((status = read(clientFd, &c, 1)) == 1)
    {
      commandBuffer[readed++] = c;
      if (c == '\n')
      {
        commandBuffer[readed] = '\0';
        LOGV("got command \"%s\"!", commandBuffer);
        std::string command = std::string(commandBuffer);
        for (auto &I: commands)
        {
          if (command == I.first)
          {
            int callbackRes = I.second();
            if (callbackRes == 0)
            {
              write(clientFd, responseOk, strlen(responseOk));
            }
            else
            {
              write(clientFd, responseError, strlen(responseError));
            }

            break;
          }
        }

        readed = 0;
      }
    }
    else if (status == -1)
    {
      LOGV("Unable to read data: %s!", strerror(errno));
      return -1;
    }
    else
    {
      usleep(500 * 1000);
    }
  }

  return 0;
}

void ControlReader::stop()
{
  __sync_bool_compare_and_swap(&shouldStop, 0, 1);
}
