//
// Created by k.leyfer on 27.09.2017.
//

#include <sstream>
#include <sys/stat.h>

#include "MotionFileWriter.h"
#include "../dirty/common/logging.h"

std::string MotionFileWriter::getFileName(nsecs_t when)
{
  std::stringstream fileNameStream;
  fileNameStream << logDirAbsPath << "/" << fileBasename << "_" << when << ".log";
  return fileNameStream.str();
}

void MotionFileWriter::writeMotionEvent(nsecs_t when, int action, int32_t changedId,
                                        uint32_t numPointers,
                                        const PointerCoords* coords,
                                        const PointerProperties* properties)
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
