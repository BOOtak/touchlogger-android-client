//
// Created by k.leyfer on 12.10.2017.
//

#ifndef TOUCHLOGGER_DIRTY_EXEC_UTILS_H
#define TOUCHLOGGER_DIRTY_EXEC_UTILS_H

#include "../dirty/common/common.h"

API_METHOD int run_child_process(const char* path, const char** args, int* in_fd, int* out_fd);

API_METHOD int
run_child_process_with_timeout(const char* path, const char** args, int* in_fd, int* out_fd,
                               long timeout_ms);

#endif //TOUCHLOGGER_DIRTY_EXEC_UTILS_H
