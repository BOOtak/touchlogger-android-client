//
// Created by k.leyfer on 27.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_MOTIONFILEWRITER_H
#define TOUCHLOGGER_DIRTY_MOTIONFILEWRITER_H

#include <string>
#include "common.h"
#include "utils/PointerCoords.h"
#include "utils/PointerProperties.h"

class MotionFileWriter
{
public:
    MotionFileWriter(std::string logDir);

    ~MotionFileWriter();

    void
    writeMotionEvent(nsecs_t when, int action, int32_t changedId, uint32_t numPointers,
                         const PointerCoords* coords, const PointerProperties* properties);

private:
    const std::string fileBasename = std::string("gestures_data");
    FILE* currentLogFile;
    std::string logDirAbsPath;
    uint32_t writtenGestures;
    const uint32_t maxWrittenGestures = 1000;

    std::string getFileName(nsecs_t when);
};


#endif //TOUCHLOGGER_DIRTY_MOTIONFILEWRITER_H
