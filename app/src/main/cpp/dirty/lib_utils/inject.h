//
// Created by k.leyfer on 22.06.2017.
//

#ifndef TOUCHLOGGER_DIRTY_INJECT_H
#define TOUCHLOGGER_DIRTY_INJECT_H

#include "../common/common.h"

API_METHOD int inject_dependency_into_library(const char* path, const char* dependency_name);

API_METHOD int replace_dependency_in_binary(const char* path, const char* old_dependency,
                                            const char* new_dependency);

#endif //TOUCHLOGGER_DIRTY_INJECT_H
