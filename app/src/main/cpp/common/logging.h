//
// Created by k.leyfer on 22.06.2017.
//

#ifndef TOUCHLOGGER_DIRTY_LOGGING_H
#define TOUCHLOGGER_DIRTY_LOGGING_H

#include <android/log.h>
#include <stdio.h>
#include <api_method.h>
#include <constants.h>

#define LOG_FILE TOUCHLOGGER_DIR "/exec_payload.log"

API_METHOD int init_log_file(const char* path);

API_METHOD void log_to_file(const char* format, ...);

#define LOGV(...) {\
  __android_log_print(ANDROID_LOG_INFO, "touchlogger-native", __VA_ARGS__);\
  printf(__VA_ARGS__);\
  printf("\n");\
  fflush(stdout);\
  log_to_file(__VA_ARGS__);\
}

#endif //TOUCHLOGGER_DIRTY_LOGGING_H
