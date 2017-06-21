//
// Created by k.leyfer on 21.06.2017.
//

#ifndef TOUCHLOGGER_DIRTY_COMMON_H
#define TOUCHLOGGER_DIRTY_COMMON_H

#include <err.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#ifdef DEBUG

#include <android/log.h>

#define LOGV(...) { __android_log_print(ANDROID_LOG_INFO, "dirtycopy", __VA_ARGS__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#define LOGV(...)
#endif

#define LOOP   0x1000000
#define TIMEOUT 1000

struct mem_arg
{
  void* offset;
  void* patch;
  size_t patch_size;
  const char* fname;
  volatile int stop;
  volatile int success;
};

void exploit(struct mem_arg* mem_arg);

int open_file(const char* path, int* fd, size_t* size);

int prepare_dirty_copy(const char* src_path, const char* dst_path, struct mem_arg* mem_arg);

#endif //TOUCHLOGGER_DIRTY_COMMON_H
