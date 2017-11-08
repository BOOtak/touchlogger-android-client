//
// Created by k.leyfer on 12.10.2017.
//

#ifndef TOUCHLOGGER_DIRTY_PAYLOAD_UTILS_H
#define TOUCHLOGGER_DIRTY_PAYLOAD_UTILS_H

#include "../dirty/common/common.h"

#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/exec_payload"
#define EXEC_PAYLOAD_DST_PATH "/data/local/tmp/exec_payload"

API_METHOD int copy_payload();

#endif //TOUCHLOGGER_DIRTY_PAYLOAD_UTILS_H
