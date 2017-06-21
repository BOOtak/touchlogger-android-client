//
// Created by k.leyfer on 21.06.2017.
//

#include <malloc.h>
#include "dirty_inject.h"
#include "../elf_parser.h"

int inject_dependency_into_library(const char *path, const char *dependency_name)
{
  struct mem_arg mem_arg;
  if (prepare_dirty_copy(path, path, &mem_arg) == -1)
  {
    LOGV("Error preparing files!");
    return -1;
  }

  struct strtab_entry soname = {
      .type = NULL,
      .value = NULL
  };
  struct dependencies_info dependencies;
  get_strtable_values(mem_arg.patch, &dependencies, &soname);
  if (soname.value != NULL)
  {
    *soname.type = DT_NEEDED;
    strcpy(soname.value, dependency_name);
    LOGV("soname: %lu: %s", *soname.type, soname.value);
  }
  else
  {
    LOGV("Unable to locate soname!");
    return 0;
  }

  free(dependencies.entries);
  exploit(&mem_arg);
  return 0;
}

int replace_dependency_in_binary(const char* path, const char* old_dependency, const char* new_dependency)
{
  struct mem_arg mem_arg;
  if (prepare_dirty_copy(path, path, &mem_arg) == -1)
  {
    LOGV("Error preparing files!");
    return -1;
  }

  struct strtab_entry soname = {
      .type = NULL,
      .value = NULL
  };
  struct dependencies_info dependencies;
  get_strtable_values(mem_arg.patch, &dependencies, &soname);

  for (int i = 0; i < dependencies.size; ++i) {
    char* needed = dependencies.entries[i].value;
    LOGV("needed: %s", needed);
    if (strcmp(needed, old_dependency) == 0)
    {
      LOGV("Let's patch it");
      strcpy(needed, new_dependency);
    }
  }

  free(dependencies.entries);
  exploit(&mem_arg);
  return 0;
}
