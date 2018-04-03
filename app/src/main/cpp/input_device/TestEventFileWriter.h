//
// Created by kirill on 29.11.17.
//

#ifndef TOUCHLOGGER_DIRTY_TESTEVENTFILEWRITER_H
#define TOUCHLOGGER_DIRTY_TESTEVENTFILEWRITER_H


#include "EventFileWriter.h"

class TestEventFileWriter : public EventFileWriter
{
public:
  TestEventFileWriter(const std::string &logDir);

private:
  void writeEvent(nsecs_t when, const std::string &event) override;
};


#endif //TOUCHLOGGER_DIRTY_TESTEVENTFILEWRITER_H
