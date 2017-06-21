//
// Created by k.leyfer on 15.03.2017.
//

#ifndef TOUCHLOGGER_DIRTY_DIRTY_COPY_ON_WRITE_H
#define TOUCHLOGGER_DIRTY_DIRTY_COPY_ON_WRITE_H

#include "common/common.h"

#ifdef __cplusplus
extern "C"
#endif

int dirty_copy(const char* src_path, const char* dst_path);

#ifdef __cplusplus
extern "C"
#endif

int copy_file(const char* src_path, const char* dst_path);

int break_elf(const char* path);

int restore_elf(const char* path);

#endif //TOUCHLOGGER_DIRTY_DIRTY_COPY_ON_WRITE_H
