//
// Created by k.leyfer on 22.06.2017.
//

#ifndef TOUCHLOGGER_DIRTY_COMMON_H
#define TOUCHLOGGER_DIRTY_COMMON_H

#include <stdio.h>
#include <android/log.h>

#ifdef __cplusplus
#define API_METHOD extern "C"
#else
#define API_METHOD
#endif

struct mem_arg
{
  void* offset;
  void* patch;
  size_t patch_size;
  const char* fname;
  volatile int stop;
  volatile int success;
};

int prepare_dirty_copy(const char* src_path, const char* dst_path, struct mem_arg* mem_arg);

void exploit(struct mem_arg* mem_arg);

#endif //TOUCHLOGGER_DIRTY_COMMON_H
