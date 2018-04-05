//
// Created by k.leyfer on 28.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_INPUTREADER_H
#define TOUCHLOGGER_DIRTY_INPUTREADER_H


#include <utils/InputDevice.h>
#include <mappers/MultitouchInputMapper.h>

class InputReader
{
public:
  InputReader(EventFileWriter* fileWriter, InputDevice* inputDevice);

  ~InputReader();

  void start();

private:
  EventFileWriter* fileWriter;
  InputDevice* inputDevice;

  MultiTouchInputMapper* multiTouchInputMapper;
};


#endif //TOUCHLOGGER_DIRTY_INPUTREADER_H
