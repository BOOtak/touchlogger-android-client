//
// Created by k.leyfer on 21.06.2017.
//

#include <unistd.h>
#include "libdirty/elf_breaker.h"

int main(int argc, const char** argv)
{
  if (daemon(0, 0) == -1)
  {
    LOGV("Unable to daemonize process: %s!", strerror(errno));
  }

  while (1)
  {
    int fd;
    if ((fd = open("/dev/input/event0", O_RDONLY)) == -1)
    {
      LOGV("Unable to open input device: %s", strerror(errno));
      break;
    }
    else
    {
      LOGV("Open input device success!");
      close(fd);
    }

    sleep(1);
  }
}
