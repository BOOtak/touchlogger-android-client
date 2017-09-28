//
// Created by k.leyfer on 08.09.2017.
//

#include <stdio.h>
#include "InputReader.h"
#include "../dirty/common/logging.h"

int main(int argc, const char** argv)
{
  InputReader* inputReader = new InputReader();
  inputReader->start();
  LOGV("Finish inputReader...");
  delete(inputReader);
  return 0;
}
