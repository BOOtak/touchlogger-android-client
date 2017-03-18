#include <jni.h>
#include <string>

#include "dirty_copy.h"
#include "file_utils.h"

JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_NativeUtils_nativeCopy(JNIEnv *env, jclass type,
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

JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_NativeUtils_injectDependencyIntoLinrary(
    JNIEnv *env, jclass type, jstring path_, jstring dependencyName_)
{
  const char *path = env->GetStringUTFChars(path_, 0);
  const char *dependencyName = env->GetStringUTFChars(dependencyName_, 0);

  int res = inject_dependency_into_library(path, dependencyName);

  env->ReleaseStringUTFChars(path_, path);
  env->ReleaseStringUTFChars(dependencyName_, dependencyName);

  return (jboolean) (res == 0);
}

JNIEXPORT jboolean JNICALL
Java_org_leyfer_thesis_touchlogger_1dirty_utils_NativeUtils_replaceDependencyInBinary(
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
Java_org_leyfer_thesis_touchlogger_1dirty_utils_NativeUtils_dirtyCopy(
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
Java_org_leyfer_thesis_touchlogger_1dirty_utils_NativeUtils_stringFromJNI(JNIEnv *env,
                                                                          jobject instance)
{
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
