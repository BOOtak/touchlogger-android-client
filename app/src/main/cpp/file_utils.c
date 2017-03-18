//
// Created by kirill on 19.03.17.
//

#include <fcntl.h>
#include "file_utils.h"
#include "dirty_copy.h"

int copy_file(const char* src_path, const char* dst_path)
{
  int src_fd = open(src_path, O_RDONLY);
  if (src_fd == -1)
  {
    LOGV("unable to open %s: %s", src_path, strerror(errno));
  }
  int dst_fd = open(dst_path, O_WRONLY | O_CREAT);
  if (dst_fd == -1)
  {
    LOGV("unable to open %s: %s", dst_path, strerror(errno));
  }

  int bufsize = 4096;
  void* buf = (void*)malloc(bufsize);
  int readed = 0;
  do {
    readed = read(src_fd, buf, bufsize);
    if (readed > 0)
    {
      int written = write (dst_fd, buf, readed);
      LOGV("written %d", written);
      if (written == -1)
      {
        LOGV("unable to write: %s", strerror(errno));
        return -1;
      }
    }

    LOGV("%d readed", readed);
  } while(readed > 0);

  close(src_fd);
  close(dst_fd);
  return 0;
}