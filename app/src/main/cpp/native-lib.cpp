#include <jni.h>
#include <string>

#include "dirty_copy.h"

extern "C"
JNIEXPORT void JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_activity_MainActivity_dirtyCopy(JNIEnv *env, jclass type,
                                                                          jstring srcPath_,
                                                                          jstring dstPath_) {
  const char *srcPath = env->GetStringUTFChars(srcPath_, 0);
  const char *dstPath = env->GetStringUTFChars(dstPath_, 0);

  dirty_copy(srcPath, dstPath);

  env->ReleaseStringUTFChars(srcPath_, srcPath);
  env->ReleaseStringUTFChars(dstPath_, dstPath);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_activity_MainActivity_stringFromJNI(JNIEnv *env,
                                                                              jobject instance) {
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
