//
// Created by k.leyfer on 15.03.2017.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <dlfcn.h>

#ifdef DEBUG

#include <android/log.h>
#include <errno.h>

#define LOGV(...) { __android_log_print(ANDROID_LOG_INFO, "dirty_load", __VA_ARGS__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#define LOGV(...)
#endif

typedef int getcon_t(char **con);

typedef int setcon_t(const char *con);

__attribute__((constructor)) void say_hello()
{

  LOGV("%d: hi (%d)", getpid(), getuid());

  int orig_pid = getpid();
  pid_t pid = fork();
  if (pid == 0)
  {
    LOGV("fork %d->%d", pid, orig_pid);

    if (getuid() != 0)
    {
      LOGV("running as uid %d\n", getuid());
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

      gid_t groups[] = {1004}; //input
      if (setgroups(sizeof(groups) / sizeof(groups[0]), groups) != 0)
      {
        LOGV("Unable to set groups: %s", strerror(errno));
        exit(0);
      }
      else
      {
        LOGV("Set groups OK!");
      }
    }

    dlerror();
#ifdef __aarch64__
    void * selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
#else
    void *selinux = dlopen("/system/lib/libselinux.so", RTLD_LAZY);
#endif
    if (selinux)
    {
      LOGV("Set groups OK!");
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
        LOGV("%d %s", ret, secontext);
        void *setcon = dlsym(selinux, "setcon");
        const char *error = dlerror();
        if (error)
        {
          LOGV("dlsym setcon error %s", error);
        }
        else
        {
          setcon_t *setcon_p = (setcon_t *) setcon;
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
      LOGV("Got root");
      int fd;
      if ((fd = open("/dev/input/event0", O_RDONLY)) == -1)
      {
        LOGV("Unable to open input device: %s", strerror(errno));
      }
      else
      {
        LOGV("Open input device success!");
        close(fd);
      }

      if ((fd = open("/sbin/adbd", O_RDONLY)) == -1)
      {
        LOGV("Unable to open adbd: %s", strerror(errno));
      }
      else
      {
        LOGV("Open adbd Success!");
        close(fd);
      }
    }

    exit(0);
  }
}
