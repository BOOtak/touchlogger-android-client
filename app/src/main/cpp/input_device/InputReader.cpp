//
// Created by k.leyfer on 28.09.2017.
//

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include "InputReader.h"
#include "../dirty/common/logging.h"

InputReader::InputReader(EventFileWriter* fileWriter, InputDevice* inputDevice)
    : inputDevice(inputDevice), multiTouchInputMapper(NULL), fileWriter(fileWriter), isPaused(false)
{}

void InputReader::start()
{
  multiTouchInputMapper = new MultiTouchInputMapper(inputDevice, fileWriter);
  multiTouchInputMapper->configure();

  //FIXME: add timestamp
  multiTouchInputMapper->reset(0);

  size_t buffer_size = 16;  // events
  input_event read_buffer[buffer_size];
  while (1)
  {
    ssize_t read_size = inputDevice->read(read_buffer, buffer_size);
    if (read_size < 0)
    {
      LOGV("Unable to read: %s!", strerror(errno));
      inputDevice->close();
      return;
    }

    int i;
    int read_items = read_size / sizeof(input_event);
    for (i = 0; i < read_items; ++i)
    {
      input_event event = read_buffer[i];
      if (__sync_bool_compare_and_swap(&isPaused, 0, 0))
      {
        multiTouchInputMapper->process(&event);
      }
      else
      {
        // drop input event
      }
    }
  }
}

void InputReader::pause()
{
  __sync_bool_compare_and_swap(&isPaused, 0, 1);
}

void InputReader::resume()
{
  __sync_bool_compare_and_swap(&isPaused, 1, 0);
}

InputReader::~InputReader()
{
  if (multiTouchInputMapper != NULL)
  {
    multiTouchInputMapper->reset(0);
    delete (multiTouchInputMapper);
    multiTouchInputMapper = NULL;
  }
}
