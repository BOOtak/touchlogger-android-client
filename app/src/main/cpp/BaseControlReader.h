//
// Created by kirill on 09.06.20.
//

#ifndef TOUCHLOGGER_DIRTY_BASECONTROLREADER_H
#define TOUCHLOGGER_DIRTY_BASECONTROLREADER_H


#include <string>
#include <map>

typedef int (* control_callback)();

class BaseControlReader
{
public:
  BaseControlReader(std::map<std::string, control_callback> commands);

  void start();

  void stop();

protected:
  int shouldStop;

  const std::map<std::string, control_callback> commands;

private:

  virtual int startServerThread() = 0;
};


#endif //TOUCHLOGGER_DIRTY_BASECONTROLREADER_H
