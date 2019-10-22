//
// Created by kirill on 14.11.17.
//

#include <sys/socket.h>
#include <linux/in.h>
#include <endian.h>
#include <unistd.h>
#include <cstring>
#include <utility>
#include "ControlReader.h"
#include "common/logging.h"

ControlReader::ControlReader(int port, std::map<std::string, control_callback> commands)
    : port(port), commands(std::move(commands)), sockFd(-1), shouldStop(0)
{}

void ControlReader::start()
{
  startServerThread();
}

int ControlReader::startServerThread()
{
  pthread_t server_thread = -1;
  int res = pthread_create(&server_thread, nullptr, &serverRoutine, (void*) this);
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
  auto* cls = reinterpret_cast<ControlReader*>(instance);

  int sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (sock_fd == -1)
  {
    LOGV("Unable to create TCP socket: %s!", strerror(errno));
    return nullptr;
  }

  struct sockaddr_in sockaddr{};
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(cls->port);

  if (bind(sock_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) == -1)
  {
    LOGV("Unable to bind on port :%d: %s!", cls->port, strerror(errno));
    return nullptr;
  }

  printf("Listening for connections...\n");
  if (listen(sock_fd, 1) == -1)
  {
    LOGV("Unable to listen: %s!", strerror(errno));
    close(sock_fd);
    return nullptr;
  }

  LOGV("Waiting for connection...");
  struct sockaddr_in client_addr{};
  socklen_t size = sizeof(client_addr);

  while (__sync_bool_compare_and_swap(&(cls->shouldStop), 0, 0))
  {
    int acceptedConnection = accept(sock_fd, (struct sockaddr*) &client_addr, &size);
    if (acceptedConnection == -1)
    {
      LOGV("Unable to accept connection: %s!", strerror(errno));
      close(sock_fd);
      return nullptr;
    }
    else
    {
      LOGV("Accepted connection!");
    }

    pthread_t connection_thread = -1;
    auto* args = new ConnectionRoutineArgs(cls, acceptedConnection);
    int res = pthread_create(&connection_thread, nullptr, connectionRoutine, (void*) args);
    if (res == -1)
    {
      LOGV("Unable to start server: %s!", strerror(errno));
      return nullptr;
    }
  }

  return nullptr;
}

void* ControlReader::connectionRoutine(void* args)
{
  auto* cArgs = static_cast<ConnectionRoutineArgs*>(args);
  int connection = cArgs->getAcceptedConnection();
  ControlReader* cls = cArgs->getControlReaderInstance();
  const size_t commandLength = 1024;
  char commandBuffer[commandLength];
  char c;
  ssize_t status;
  int readed = 0;
  const useconds_t readTimeout = 500 * 1000;

  while (__sync_bool_compare_and_swap(&cls->shouldStop, 0, 0))
  {
    if ((status = read(connection, &c, 1)) == 1)
    {
      commandBuffer[readed++] = c;
      if (c == '\n')
      {
        commandBuffer[readed] = '\0';
        LOGV("got command \"%s\"!", commandBuffer);
        std::string command = std::string(commandBuffer);
        bool knownCommand = false;
        for (auto &I: cls->commands)
        {
          if (command == I.first)
          {
            knownCommand = true;
            int callbackRes = I.second();
            LOGV("Found callback for the command, res = %d.", callbackRes);
            const char* response;
            if (callbackRes == 0)
            {
              response = RESPONSE_OK "\n";
            }
            else
            {
              response = RESPONSE_ERROR "\n";
            }

            write(connection, response, strlen(response));
            break;
          }
        }

        if (!knownCommand)
        {
          LOGV("Unable to recognize command \"%s\"!", command.c_str());
        }

        readed = 0;
      }
    }
    else if (status == -1)
    {
      LOGV("Unable to read data: %s!", strerror(errno));
      close(connection);
      delete(cArgs);
      return nullptr;
    }
    else
    {
      usleep(readTimeout);
    }
  }

  close(connection);
  delete(cArgs);
  return nullptr;
}

void ControlReader::stop()
{
  __sync_bool_compare_and_swap(&shouldStop, 0, 1);
}
