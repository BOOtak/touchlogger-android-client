//
// Created by kirill on 05.04.18.
//

#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "Utils.h"
#include "../../common/logging.h"

InputDevice* findTouchscreen()
{
  DIR* input_device_dir = opendir(input_device_dir_path);
  if (input_device_dir == NULL)
  {
    LOGV("Unable to open %s: %s!\n", input_device_dir_path, strerror(errno));
    return nullptr;
  }

  char absPath[BUFSIZ];
  struct dirent* item;
  do
  {
    item = readdir(input_device_dir);
    if (item == NULL)
    {
      if (errno == 0)
      {
        break;
      }
      else
      {
        closedir(input_device_dir);
        LOGV("Unable to read dir: %s!", strerror(errno));
        return nullptr;
      }
    }

    if (strcmp(item->d_name, "..") == 0
        || strcmp(item->d_name, ".") == 0)
    {
      continue;
    }

    if (item->d_type == DT_CHR)
    {
      memset(absPath, 0, BUFSIZ);
      snprintf(absPath, BUFSIZ, "%s/%s", input_device_dir_path, item->d_name);

      struct stat st;
      if (stat(absPath, &st) == 0)
      {
        LOGV("Found input device at %s!", absPath);
        InputDevice* inputDevice = new InputDevice(absPath);
        if (inputDevice->configureAsTouchscreenDevice())
        {
          LOGV("Type: %d", inputDevice->getType());
          LOGV("Parameters:");
          LOGV("%6s %5s %5s %5s %5s %5s", "type", "min", "max", "flat", "fuzz", "res");
          for (std::vector<RawAbsoluteAxisInfo>::iterator it = inputDevice->getAxisInfos().begin(),
                   end = inputDevice->getAxisInfos().end(); it != end; it++)
          {
            RawAbsoluteAxisInfo info = *it;
            LOGV("0x%04x %5d %5d %5d %5d %5d", info.tested_bit, info.minValue, info.maxValue,
                 info.flat, info.fuzz, info.resolution);
          }

          closedir(input_device_dir);
          return inputDevice;
        }
      }
    }
  } while (1);

  closedir(input_device_dir);
  return nullptr;
}
