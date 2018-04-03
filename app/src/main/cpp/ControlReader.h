//
// Created by kirill on 14.11.17.
//

#ifndef TOUCHLOGGER_DIRTY_CONTROLREADER_H
#define TOUCHLOGGER_DIRTY_CONTROLREADER_H

#include <map>
#include <string>
#include <mutex>
#include "utils/net_utils.h"

typedef void (* control_callback)();

class ControlReader
{
public:

  ControlReader(int port, std::map<std::string, control_callback> &commands);

  void start();

  void stop();

private:
  int port;
  int sockFd;
  int shouldStop;

  std::map<std::string, control_callback> commands;

  int startServerThread();

  int startConnectionThread();

  static void* serverRoutine(void* instance);

  int connectionRoutine(int clientFd);
};


#endif //TOUCHLOGGER_DIRTY_CONTROLREADER_H
