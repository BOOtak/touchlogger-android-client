//
// Created by k.leyfer on 22.06.2017.
//


#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "file_utils/file_utils.h"
#include "lib_utils/inject.h"
#include "common/logging.h"
#include "file_utils/dirty_copy.h"

int main(int argc, const char* argv[])
{
  if (argc < 2)
  {
    LOGV("usage %s /data/local/tmp/default.prop /default.prop", argv[0]);
    return 0;
  }

  if (strcmp(argv[1], "-c") == 0)
  {
    LOGV("Copy %s to %s", argv[2], argv[3]);
    int res = copy_file(argv[2], argv[3]);
    if (chmod(argv[3], 0777) != 0)
    {
      LOGV("Unable to chmod: %s", strerror(errno));
    }

    return res;
  }

  if (strcmp(argv[1], "-d") == 0)
  {
    LOGV("Dirty copy %s to %s", argv[2], argv[3]);
    return dirty_copy(argv[2], argv[3]);
  }

  if (strcmp(argv[1], "-id") == 0)
  {
    LOGV("Inject dependency %s into %s", argv[3], argv[2]);
    return inject_dependency_into_library(argv[2], argv[3]);
  }

  if (strcmp(argv[1], "-rd") == 0)
  {
    LOGV("Replace dependency in %s from %s to %s", argv[2], argv[3], argv[4]);
    return replace_dependency_in_binary(argv[2], argv[3], argv[4]);
  }
}
