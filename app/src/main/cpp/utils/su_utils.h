//
// Created by k.leyfer on 12.10.2017.
//

#ifndef TOUCHLOGGER_DIRTY_SU_UTILS_H
#define TOUCHLOGGER_DIRTY_SU_UTILS_H

#include "../dirty/common/common.h"

API_METHOD int check_su_binary(const char* path);

API_METHOD int check_suid_bit(const char* path);

#endif //TOUCHLOGGER_DIRTY_SU_UTILS_H
