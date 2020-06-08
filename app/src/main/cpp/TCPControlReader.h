//
// Created by kirill on 14.11.17.
//

#ifndef TOUCHLOGGER_DIRTY_TCPCONTROLREADER_H
#define TOUCHLOGGER_DIRTY_TCPCONTROLREADER_H

#include <map>
#include <string>
#include <mutex>
#include "utils/net_utils.h"
#include "BaseControlReader.h"

class TCPControlReader : public BaseControlReader
{
public:

  TCPControlReader(int port, std::map<std::string, control_callback> commands);

private:
  int port;
  int sockFd;

  int startServerThread() override;

  static void* serverRoutine(void* instance);

  static void* connectionRoutine(void* args);
};

class ConnectionRoutineArgs
{
public:
  ConnectionRoutineArgs(TCPControlReader* instance, int acceptedConnection)
      : controlReaderInstance(instance), acceptedConnection(acceptedConnection)
  {}

private:
  TCPControlReader* controlReaderInstance;
  int acceptedConnection;
public:
  inline TCPControlReader* getControlReaderInstance()
  {
    return controlReaderInstance;
  }

  inline int getAcceptedConnection()
  {
    return acceptedConnection;
  }
};

#endif //TOUCHLOGGER_DIRTY_TCPCONTROLREADER_H
