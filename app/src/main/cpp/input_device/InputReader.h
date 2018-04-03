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
  InputReader(EventFileWriter* fileWriter);

  ~InputReader();

  void start();

private:
  EventFileWriter* fileWriter;
  InputDevice inputDevice;

  MultiTouchInputMapper* multiTouchInputMapper;

  int findTouchscreen();

  //TODO: move to InputDevice
  bool configureTouchscreenDevice(const char* input_device_path);

  bool containsNonzeroBytes(const uint8_t* array, uint32_t start_index, uint32_t end_index);
};


#endif //TOUCHLOGGER_DIRTY_INPUTREADER_H
