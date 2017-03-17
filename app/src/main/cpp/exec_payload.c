//
// Created by kirill on 17.03.17.
//

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

#include "dirty_copy.h"

typedef int getcon_t(char **con);

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
    }

    sleep(1);
  }

  return 0;
}
