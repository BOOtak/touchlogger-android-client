//
// Created by kirill on 19.03.17.
//

#ifndef TOUCHLOGGER_DIRTY_FILE_UTILS_H
#define TOUCHLOGGER_DIRTY_FILE_UTILS_H

#ifdef __cplusplus
#define API_METHOD extern "C"
#else
#define API_METHOD
#endif

API_METHOD int copy_file(const char* src_path, const char* dst_path);

API_METHOD int copy_file_with_mode(const char* src_path, const char* dst_path, int mode);

#endif //TOUCHLOGGER_DIRTY_FILE_UTILS_H
