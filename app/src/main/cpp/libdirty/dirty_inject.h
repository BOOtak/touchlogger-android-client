//
// Created by k.leyfer on 21.06.2017.
//

#ifndef TOUCHLOGGER_DIRTY_DIRTY_INJECT_H
#define TOUCHLOGGER_DIRTY_DIRTY_INJECT_H

#include "common/common.h"

#ifdef __cplusplus
extern "C"
#endif

int inject_dependency_into_library(const char* path, const char* dependency_name);

#ifdef __cplusplus
extern "C"
#endif

int replace_dependency_in_binary(const char* path, const char* old_dependency,
                                 const char* new_dependency);


#endif //TOUCHLOGGER_DIRTY_DIRTY_INJECT_H
