#include <jni.h>
#include <string>
#include <stdlib.h>

// TODO: make this depend on build type
#define DEBUG 1

#include "dirty_copy.h"
#include "file_utils.h"

extern "C"
JNIEXPORT void JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JNIAPI_restartZygote(JNIEnv *env,
                                                                     jclass type,
                                                                     jstring garbagePath_,
                                                                     jstring backupPath_)
{
  const char *garbagePath = env->GetStringUTFChars(garbagePath_, 0);
  const char *backupPath = env->GetStringUTFChars(backupPath_, 0);

  if (fork() == 0)
  {
#ifdef __aarch64__
    const char* app_process_path = "/system/bin/app_process64";
#else
    const char *app_process_path = "/system/bin/app_process32";
#endif

    if (daemon(0, 0) == -1)
    {
      LOGV("Unable to daemonize process: %s!", strerror(errno));
    }

    int killer_pid;
    if ((killer_pid = fork()) == 0)
    {
      if (dirty_copy(garbagePath, app_process_path) == -1)
      {
        LOGV("Unable to overwrite app_process (%s) with %s!", app_process_path, garbagePath);
        exit(-1);
      }
      else
      {
        LOGV("App_process successfully overwritten!");
      }

      exit(0);
    }

    //FIXME: come up with something less dumb than that
    sleep(10);

    LOGV("Zygote is probably dead now");

    if (kill(killer_pid, 9) == -1)
    {
      LOGV("Unable to kill killer thread: %s", strerror(errno));
    }
    else
    {
      LOGV("Killer process killed successfully!");
    }

    if (dirty_copy(backupPath, app_process_path) == -1)
    {
      LOGV("Unable to restore app_process from backup! Reboot yor device and try again.");
      exit(-1);
    }
    else
    {
      LOGV("Zygote successfully restarted!");
      exit(0);
    }
  }

  env->ReleaseStringUTFChars(garbagePath_, garbagePath);
  env->ReleaseStringUTFChars(backupPath_, backupPath);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JNIAPI_nativeCopy(JNIEnv *env, jclass type,
                                                                  jstring srcPath_,
                                                                  jstring dstPath_)
{
  const char *srcPath = env->GetStringUTFChars(srcPath_, 0);
  const char *dstPath = env->GetStringUTFChars(dstPath_, 0);

  int res = copy_file(srcPath, dstPath);

  env->ReleaseStringUTFChars(srcPath_, srcPath);
  env->ReleaseStringUTFChars(dstPath_, dstPath);

  return (jboolean) (res == 0);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JNIAPI_injectDependencyIntoLinrary(
    JNIEnv *env, jclass type, jstring path_, jstring dependencyName_)
{
  const char *path = env->GetStringUTFChars(path_, 0);
  const char *dependencyName = env->GetStringUTFChars(dependencyName_, 0);

  int res = inject_dependency_into_library(path, dependencyName);

  env->ReleaseStringUTFChars(path_, path);
  env->ReleaseStringUTFChars(dependencyName_, dependencyName);

  return (jboolean) (res == 0);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JNIAPI_replaceDependencyInBinary(
    JNIEnv *env, jclass type, jstring path_, jstring oldDependency_, jstring newDependency_)
{
  const char *path = env->GetStringUTFChars(path_, 0);
  const char *oldDependency = env->GetStringUTFChars(oldDependency_, 0);
  const char *newDependency = env->GetStringUTFChars(newDependency_, 0);

  int res = replace_dependency_in_binary(path, oldDependency, newDependency);

  env->ReleaseStringUTFChars(path_, path);
  env->ReleaseStringUTFChars(oldDependency_, oldDependency);
  env->ReleaseStringUTFChars(newDependency_, newDependency);

  return (jboolean) (res == 0);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JNIAPI_dirtyCopy(
    JNIEnv *env, jclass type,
    jstring srcPath_, jstring dstPath_)
{
  const char *srcPath = env->GetStringUTFChars(srcPath_, 0);
  const char *dstPath = env->GetStringUTFChars(dstPath_, 0);

  int res = dirty_copy(srcPath, dstPath);

  env->ReleaseStringUTFChars(srcPath_, srcPath);
  env->ReleaseStringUTFChars(srcPath_, dstPath);

  return (jboolean) (res == 0);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JNIAPI_stringFromJNI(JNIEnv *env,
                                                                     jobject instance)
{
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
