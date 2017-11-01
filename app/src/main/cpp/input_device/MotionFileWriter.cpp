//
// Created by k.leyfer on 27.09.2017.
//

#include <sstream>
#include <sys/stat.h>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "MotionFileWriter.h"
#include "../dirty/common/logging.h"

std::string MotionFileWriter::getFileName(nsecs_t when)
{
  std::stringstream fileNameStream;
  fileNameStream << logDirAbsPath << "/" << fileBasename << "_" << when << ".log";
  return fileNameStream.str();
}

void MotionFileWriter::checkFile(nsecs_t when)
{
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
  else if (writtenGestures > maxWrittenGestures)
  {
    LOGV("Gestures limit exceeded, create new file");
    if (fclose(currentLogFile) != 0)
    {
      LOGV("Unable to close file, try next time");
    }
    else
    {
      currentLogFile = fopen(getFileName(when).c_str(), "a+");
      writtenGestures = 0;
    }
  }
}

void MotionFileWriter::writeMotionEvent(nsecs_t when, int action, int32_t changedId,
                                        uint32_t numPointers,
                                        const PointerCoords* coords,
                                        const PointerProperties* properties)
{
  checkFile(when);
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

  fprintf(currentLogFile, "{\"ts\": %llu,"
              "\"prefix\": \"%s\","
              "\"pointer_count\": %d,"
              "\"changed_id\": %d,"
              "\"pointers\": [",
          when, actionStr, numPointers, changedId);

  for (int i = 0; i < numPointers; ++i)
  {
    fprintf(currentLogFile, "{\"id\": %d, \"x\": %f, \"y\": %f, \"pressure\": %f}",
            properties[i].id, coords[i].getAxisValue(AMOTION_EVENT_AXIS_X),
            coords[i].getAxisValue(AMOTION_EVENT_AXIS_Y),
            coords[i].getAxisValue(AMOTION_EVENT_AXIS_PRESSURE));

    if (i != numPointers - 1)
    {
      fputs(",", currentLogFile);
    }
  }

  fputs("]}\n", currentLogFile);
  writtenGestures++;
  fflush(currentLogFile);
}

int MotionFileWriter::runChildProcess(const char *path, const char **args, int *inFd, int *outFd)
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

    if (execv(path, (char *const *) args) == -1)
    {
      LOGV("Unable to execute %s: %s!", path, strerror(errno));
    }

    exit(-1);
  }
  else
  {
    *inFd = pipeIn[1];
    *outFd = pipeOut[0];
    return 0;
  }
}

std::string MotionFileWriter::findCurrentFocusWindow()
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
  char window[4096];

  int status = 0;
  if (waitpid(pid, &status, 0) == -1)
  {
    LOGV("Unable to wait for process: %s!", strerror(errno));
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
//      mCurrentFocus=Window{d192c75 u0 StatusBar}
      if (strstr(buf, "mCurrentFocus") != NULL)
      {
        char* u0Index = strstr(buf, "u0");
        if (u0Index != NULL)
        {
          memset(window, 0, 4096);
          if (sscanf(u0Index, "u0 %s}", window) == 1)
          {
            char* index;
            if ((index = strrchr(window, '}')) != NULL)
            {
              *index = '\0';
            }

            close(in);
            close(out);
            return std::string(window);
          }
        }
        else
        {
          return std::string("null");
        }
      }
    }
  }

  close(in);
  close(out);
  return std::string("");
}

void MotionFileWriter::writeCurrentFocusWindow(nsecs_t when)
{
  checkFile(when);
  std::string window = findCurrentFocusWindow();
  if (!window.empty())
  {
    fprintf(currentLogFile, "{\"ts\": %llu, \"window\": \"%s\"}\n", when, window.c_str());
    fflush(currentLogFile);
  }
}

MotionFileWriter::MotionFileWriter(std::string logDir) : logDirAbsPath(logDir),
                                                         currentLogFile(NULL),
                                                         writtenGestures(0)
{}

MotionFileWriter::~MotionFileWriter()
{
  if (currentLogFile != NULL)
  {
    fclose(currentLogFile);
    currentLogFile = NULL;
  }
}
