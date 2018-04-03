//
// Created by kirill on 29.11.17.
//

#include "TestEventFileWriter.h"

TestEventFileWriter::TestEventFileWriter(const std::string &logDir) : EventFileWriter(logDir)
{}

void TestEventFileWriter::writeEvent(nsecs_t when, const std::string &event)
{
  EventFileWriter::writeEvent(when, event);
  printf(".");
  fflush(stdout);
}
