//
// Created by k.leyfer on 12.10.2017.
//

#include "payload_utils.h"


#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../dirty/common/logging.h"
#include "../dirty/file_utils/file_utils.h"

int copy_payload()
{
  bool should_copy = false;
  struct stat exec_payload_dst_stat, exec_payload_sdcard_stat;
  if (stat(EXEC_PAYLOAD_DST_PATH, &exec_payload_dst_stat) == -1)
  {
    if (errno == ENOENT)
    {
      LOGV("Payload at "
               EXEC_PAYLOAD_DST_PATH
               " not found, copy.");
      should_copy = true;
    }
    else
    {
      LOGV("Unable to get info about payload at "
               EXEC_PAYLOAD_DST_PATH
               ", exit.");
      return -1;
    }
  }
  else if (stat(EXEC_PAYLOAD_SDCARD_PATH, &exec_payload_sdcard_stat) == 0)
  {
    if (exec_payload_dst_stat.st_mtim.tv_sec <= exec_payload_sdcard_stat.st_mtim.tv_sec &&
        exec_payload_dst_stat.st_mtim.tv_nsec < exec_payload_sdcard_stat.st_mtim.tv_nsec)
    {
      LOGV("Payload at "
               EXEC_PAYLOAD_SDCARD_PATH
               " is newer than "
               EXEC_PAYLOAD_DST_PATH
               ", copy.");
      should_copy = true;
    }
  }
  else
  {
    LOGV("Unable to open %s: %s!", EXEC_PAYLOAD_SDCARD_PATH, strerror(errno));
    return -1;
  }

  if (should_copy)
  {
    LOGV("Copy payload from /sdcard");
    if (copy_file_with_mode(EXEC_PAYLOAD_SDCARD_PATH, EXEC_PAYLOAD_DST_PATH, 0777) == -1)
    {
      LOGV("Unable to copy %s to %s!", EXEC_PAYLOAD_SDCARD_PATH, EXEC_PAYLOAD_DST_PATH);
      return -1;
    }
  }

  return 0;
}
