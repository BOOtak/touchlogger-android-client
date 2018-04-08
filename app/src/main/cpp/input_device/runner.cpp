//
// Created by k.leyfer on 08.09.2017.
//

#include <stdio.h>
#include <utils/Utils.h>
#include "InputReader.h"
#include "../dirty/common/logging.h"
#include "TestEventFileWriter.h"

int main(int argc, const char** argv)
{
  EventFileWriter* testEventFileWriter = new TestEventFileWriter("");
  InputDevice* touchscreen = findTouchscreen();
  InputReader* inputReader = new InputReader(testEventFileWriter, touchscreen);
  inputReader->start();
  LOGV("Finish inputReader...");
  delete(inputReader);
  return 0;
}
