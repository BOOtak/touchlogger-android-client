//
// Created by kirill on 19.03.17.
//

#ifndef TOUCHLOGGER_DIRTY_FILE_UTILS_H
#define TOUCHLOGGER_DIRTY_FILE_UTILS_H

#include "../common/common.h"
#include "../common/logging.h"

API_METHOD int copy_file(const char* src_path, const char* dst_path);

API_METHOD int copy_file_with_mode(const char* src_path, const char* dst_path, mode_t mode);

#endif //TOUCHLOGGER_DIRTY_FILE_UTILS_H
