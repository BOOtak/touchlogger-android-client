//
// Created by k.leyfer on 27.09.2017.
//

#include <sstream>
#include <sys/stat.h>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "EventFileWriter.h"
#include "../dirty/common/logging.h"

#define USECS_IN_SECS 1000LL * 1000LL * 1000LL

std::string EventFileWriter::getFileName(nsecs_t when)
{
  std::stringstream fileNameStream;
  fileNameStream << logDirAbsPath << "/" << fileBasename << "_" << when << ".log";
  return fileNameStream.str();
}

void EventFileWriter::writeMotionEvent(nsecs_t when, int action, int32_t changedId,
                                       uint32_t numPointers,
                                       const PointerCoords* coords,
                                       const PointerProperties* properties)
{
  const char* actionStr;
  switch (action)
  {
    case AMOTION_EVENT_ACTION_DOWN:
      actionStr = "Down";
      break;
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
      actionStr = "Pointer down";
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      actionStr = "Move";
      break;
    case AMOTION_EVENT_ACTION_POINTER_UP:
      actionStr = "Pointer up";
      break;
    case AMOTION_EVENT_ACTION_UP:
      actionStr = "Up";
      break;
    default:
      actionStr = "Unknown";
  }

  std::stringstream stream;

  stream << "{\"ts\": " << when << ", "
         << "\"prefix\": \"" << actionStr << "\", "
         << "\"pointer_count\": " << numPointers << ", "
         << "\"changed_id\": " << changedId << ", "
         << "\"pointers\": [";

  for (int i = 0; i < numPointers; ++i)
  {
    stream << "{\"id\": " << properties[i].id << ", "
           << "\"x\": " << coords[i].getAxisValue(AMOTION_EVENT_AXIS_X) << ", "
           << "\"y\": " << coords[i].getAxisValue(AMOTION_EVENT_AXIS_Y) << ", "
           << "\"pressure\": " << coords[i].getAxisValue(AMOTION_EVENT_AXIS_PRESSURE)
           << "}";
    if (i != numPointers - 1)
    {
      stream << ", ";
    }
  }

  stream << "]}\n";
  writeEvent(when, stream.str());
}

int EventFileWriter::runChildProcess(const char* path, const char** args, int* inFd, int* outFd)
{
  int pipeIn[2];
  int pipeOut[2];

  if (pipe(pipeIn) == -1)
  {
    LOGV("Unable to pipe in: %s!", strerror(errno));
    return -1;
  }

  if (pipe(pipeOut) == -1)
  {
    LOGV("Unable to pipe out: %s!", strerror(errno));
    return -1;
  }

  int pid = fork();
  if (pid == -1)
  {
    LOGV("Unable to fork: %s!", strerror(errno));
    return -1;
  }

  if (pid == 0)
  {
    dup2(pipeIn[0], STDIN_FILENO);
    dup2(pipeOut[1], STDOUT_FILENO);
    dup2(pipeOut[1], STDERR_FILENO);

    if (execv(path, (char* const*) args) == -1)
    {
      LOGV("Unable to execute %s: %s!", path, strerror(errno));
    }

    exit(-1);
  }
  else
  {
    close(pipeOut[1]);
    close(pipeIn[0]);
    *inFd = pipeIn[1];
    *outFd = pipeOut[0];
    return 0;
  }
}

std::string EventFileWriter::parseFocusedWindowString(const char* focusedWindowString)
{
//  mCurrentFocus=Window{d192c75 u0 StatusBar}
//  or
//  mCurrentFocus=null
//  or
//  mCurrentFocus=Window{42d3cd18 u0 com.cooee.phenix/com.cooee.phenix.Launcher}
//  or
//  mCurrentFocus=Window{42310f20 com.test/com.test.MainActivity paused=false}
  char window[4096];
  memset(window, 0, 4096);
  const char* windowIndex = strstr(focusedWindowString, "Window{");
  if (windowIndex != NULL)
  {
    const char* u0Index = NULL;
    if ((u0Index = strstr(focusedWindowString, "u0")) != NULL)
    {
      if (sscanf(u0Index, "u0 %s", window) == 1)
      {
        char* index;
        if ((index = strrchr(window, '}')) != NULL)
        {
          *index = '\0';
        }

        return std::string(window);
      }
    }
    else
    {
      long addr = -1;
      if (sscanf(windowIndex, "Window{%lx %s", &addr, window) == 2)
      {
        char* index;
        if ((index = strrchr(window, '}')) != NULL)
        {
          *index = '\0';
        }

        return std::string(window);
      }
    }
  }

  return std::string("null");
}

std::string EventFileWriter::findCurrentFocusWindow()
{
  int in = -1, out = -1;
  const char* path = "/system/bin/dumpsys";
  const char* args[] = {"/system/bin/dumpsys", "window", "windows", (char*) NULL};
  int pid = runChildProcess(path, args, &in, &out);
  if (pid == -1)
  {
    LOGV("Unable to launch dumpsys");
    return std::string("");
  }

  char buf[4096];
  char c;
  int readed = 0;

  int status = 0;
  if (waitpid(pid, &status, 0) == -1)
  {
    LOGV("Unable to wait for process: %s!", strerror(errno));
    close(in);
    close(out);
    return std::string("");
  }

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
  {
    LOGV("Abnormal process exit!");
  }

  while (read(out, &c, 1) == 1)
  {
    buf[readed++] = c;
    if (c == '\n')
    {
      buf[readed] = '\0';
      readed = 0;

      if (strstr(buf, "mCurrentFocus") != NULL)
      {
        close(in);
        close(out);
        return parseFocusedWindowString(buf);
      }
    }
  }

  close(in);
  close(out);
  return std::string("");
}

void EventFileWriter::writeHeartBeat(nsecs_t when)
{
  std::stringstream stream;
  stream << "{\"ts\": " << when << ", \"online\": \"true\"}\n";
  writeEvent(when, stream.str());
}

void* EventFileWriter::heartBeatWritingRoutine(void* data)
{
  EventFileWriter* cls = reinterpret_cast<EventFileWriter*>(data);
  while (__sync_bool_compare_and_swap(&(cls->should_stop), 0, 0))
  {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    cls->writeHeartBeat(ts.tv_sec * USECS_IN_SECS + ts.tv_nsec);
    sleep(1);
  }

  return NULL;
}

void EventFileWriter::createHeartBeatThread()
{
  pthread_t heartBeatThread;
  int res;
  EventFileWriter* cls = this;
  if ((res = pthread_create(&heartBeatThread, NULL, &heartBeatWritingRoutine,
                            reinterpret_cast<void*>(cls))) != 0)
  {
    LOGV("Unable to create thread: %s!\n", strerror(res));
  }
}

void EventFileWriter::writeEvent(nsecs_t when, const std::string &event)
{
  eventLock.lock();
  if (currentLogFile == NULL)
  {
    struct stat st;
    if (stat(logDirAbsPath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
    {
      LOGV("Dir for gesture data not found, create one");
      if (mkdir(logDirAbsPath.c_str(), 0755) == -1)
      {
        LOGV("Unable to create dir: %s!", strerror(errno));
        return;
      }
    }

    currentLogFile = fopen(getFileName(when).c_str(), "a+");
  }
  else if (writtenEvents > maxWrittenEvents)
  {
    LOGV("Events limit exceeded, create new file");
    if (fclose(currentLogFile) != 0)
    {
      LOGV("Unable to close file, try next time");
    }
    else
    {
      currentLogFile = fopen(getFileName(when).c_str(), "a+");
      writtenEvents = 0;
    }
  }

  fputs(event.c_str(), currentLogFile);
  fflush(currentLogFile);
  writtenEvents++;
  eventLock.unlock();
}

void EventFileWriter::writeCurrentFocusWindow(nsecs_t when)
{
  std::string window = findCurrentFocusWindow();
  if (!window.empty())
  {
    std::stringstream stream;
    stream << "{\"ts\": " << when << ", "
           << "\"window\": \"" << window.c_str()
           << "\"}\n";
    writeEvent(when, stream.str());
  }
}

EventFileWriter::EventFileWriter(std::string logDir) : logDirAbsPath(logDir),
                                                       currentLogFile(NULL),
                                                       writtenEvents(0),
                                                       fileBasename("touch_event_data"),
                                                       maxWrittenEvents(3600),
                                                       eventLock(),
                                                       should_stop(0)
{
  createHeartBeatThread();
}

EventFileWriter::~EventFileWriter()
{
  __sync_bool_compare_and_swap(&should_stop, 0, 1);
  eventLock.lock();
  if (currentLogFile != NULL)
  {
    fclose(currentLogFile);
    currentLogFile = NULL;
  }

  eventLock.unlock();
}
