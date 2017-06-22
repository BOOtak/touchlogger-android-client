//
// Created by k.leyfer on 15.03.2017.
//

#ifndef TOUCHLOGGER_DIRTY_DIRTY_COPY_ON_WRITE_H
#define TOUCHLOGGER_DIRTY_DIRTY_COPY_ON_WRITE_H

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

#ifdef __cplusplus
extern "C"
#endif

int dirty_copy(const char* src_path, const char* dst_path);

#ifdef __cplusplus
extern "C"
#endif

int inject_dependency_into_library(const char* path, const char* dependency_name);

#ifdef __cplusplus
extern "C"
#endif

int replace_dependency_in_binary(const char* path, const char* old_dependency,
                                 const char* new_dependency);

#endif //TOUCHLOGGER_DIRTY_DIRTY_COPY_ON_WRITE_H
