//
// Created by k.leyfer on 08.09.2017.
//

#include <stdio.h>
#include <linux/input.h>
#include <unistd.h>
#include "input_device_utils.h"
#include "mappers/TouchInputMapper.h"
#include "mappers/MultitouchInputMapper.h"

int main(int argc, const char** argv)
{
  InputDevice inputDevice = {0};
  if (find_touchscreen(&inputDevice) == -1)
  {
    LOGV("Unable to find touchscreen!");
    return -1;
  }

  MultiTouchInputMapper* multiTouchInputMapper = new MultiTouchInputMapper(&inputDevice);
  multiTouchInputMapper->configure();

  //FIXME: add timestamp
  multiTouchInputMapper->reset(0);

  int buffer_size = 16;  // events
  input_event read_buffer[buffer_size];
  while (1)
  {
    int read_size = read(inputDevice.fd, (void*) read_buffer, sizeof(input_event) * buffer_size);
    if (read_size < 0)
    {
      LOGV("Unable to read: %s!", strerror(errno));
      close(inputDevice.fd);
      return -1;
    }

    int i;
    int read_items = read_size / sizeof(input_event);
    for (i = 0; i < read_items; ++i)
    {
      input_event event = read_buffer[i];
//      LOGV("%lu.%lu 0x%x 0x%x 0x%x", event.time.tv_sec, event.time.tv_usec, event.code, event.type,
//           event.value);
      multiTouchInputMapper->process(&event);
    }
  }

  return 0;
}
