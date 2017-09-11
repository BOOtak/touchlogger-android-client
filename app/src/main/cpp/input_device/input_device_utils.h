//
// Created by k.leyfer on 08.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_TEST_INPUT_H
#define TOUCHLOGGER_DIRTY_TEST_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <linux/input-event-codes.h>

#include "../dirty/common/logging.h"
#include "common.h"
#include "utils/RawAbsoluteAxisInfo.h"
#include "utils/InputDevice.h"

bool containsNonzeroBytes(const uint8_t* array, uint32_t start_index, uint32_t end_index);

bool
deviceIsTouchscreen(const char* input_device_path, InputDevice* inputDevice);

int find_touchscreen(InputDevice* inputDevice);

#endif //TOUCHLOGGER_DIRTY_TEST_INPUT_H
