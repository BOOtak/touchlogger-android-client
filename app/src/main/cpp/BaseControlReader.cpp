//
// Created by kirill on 09.06.20.
//

#include "BaseControlReader.h"

BaseControlReader::BaseControlReader(std::map<std::string, control_callback> commands)
    : commands(std::move(commands)), shouldStop(0)
{}

void BaseControlReader::start()
{
  startServerThread();
}

void BaseControlReader::stop()
{
  __sync_bool_compare_and_swap(&shouldStop, 0, 1);
}
