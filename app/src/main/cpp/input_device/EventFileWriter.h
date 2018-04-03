//
// Created by k.leyfer on 27.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_EVENTFILEWRITER_H
#define TOUCHLOGGER_DIRTY_EVENTFILEWRITER_H

#include <mutex>
#include <string>

#include <common.h>
#include <utils/PointerCoords.h>
#include <utils/PointerProperties.h>

class EventFileWriter
{
public:
  EventFileWriter(std::string logDir);

  ~EventFileWriter();

  void writeMotionEvent(nsecs_t when, int action, int32_t changedId, uint32_t numPointers,
                        const PointerCoords* coords, const PointerProperties* properties);

  void writeCurrentFocusWindow(nsecs_t when);

  int should_stop;

private:
  const std::string fileBasename;
  FILE* currentLogFile;
  std::string logDirAbsPath;
  uint32_t writtenEvents;
  const uint32_t maxWrittenEvents;
  std::mutex eventLock;

  std::string getFileName(nsecs_t when);

  std::string findCurrentFocusWindow();

  int runChildProcess(const char* path, const char** args, int* inFd, int* outFd);

  std::string parseFocusedWindowString(const char* focusedWindowString);

protected:
  virtual void writeEvent(nsecs_t when, const std::string &event);
};


#endif //TOUCHLOGGER_DIRTY_EVENTFILEWRITER_H
