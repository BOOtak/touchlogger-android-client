//
// Created by kirill on 14.11.17.
//

#ifndef TOUCHLOGGER_DIRTY_CONTROLREADER_H
#define TOUCHLOGGER_DIRTY_CONTROLREADER_H

#include <map>
#include <string>
#include <mutex>
#include "utils/net_utils.h"

typedef int (* control_callback)();

class ControlReader
{
public:

  ControlReader(int port, const std::map<std::string, control_callback> &commands);

  void start();

  void stop();

private:
  int port;
  int sockFd;
  int shouldStop;

  const std::map<std::string, control_callback> commands;

  int startServerThread();

  static void* serverRoutine(void* instance);

  static void* connectionRoutine(void* args);
};

class ConnectionRoutineArgs
{
public:
  ConnectionRoutineArgs(ControlReader* instance, int acceptedConnection)
      : controlReaderInstance(instance), acceptedConnection(acceptedConnection)
  {}

private:
  ControlReader* controlReaderInstance;
  int acceptedConnection;
public:
  inline ControlReader* getControlReaderInstance()
  {
    return controlReaderInstance;
  }

  inline int getAcceptedConnection()
  {
    return acceptedConnection;
  }
};

#endif //TOUCHLOGGER_DIRTY_CONTROLREADER_H
