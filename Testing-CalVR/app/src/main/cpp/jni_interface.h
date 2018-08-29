#ifndef TESTING_JNI_INTERFACE_H_
#define TESTING_JNI_INTERFACE_H_

#include <jni.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_example_kroth_testing_JniInterface_##method_name

// Helper functions to provide access to Java from C via JNI.
extern "C" {
    JNI_METHOD(jlong, createNativeApplication)(JNIEnv *env, jclass, jobject j_asset_manager);
    JNI_METHOD(void, destroyNativeApplication)(JNIEnv *, jclass, jlong native_application);

    JNI_METHOD(void, onPause)(JNIEnv *, jclass, jlong native_application);
    JNI_METHOD(void, onResume)(JNIEnv *env, jclass, jlong native_application, jobject context, jobject activity);

    JNI_METHOD(void, onGlSurfaceCreated)(JNIEnv *, jclass, jlong native_application);
    JNI_METHOD(void, onDisplayGeometryChanged)(JNIEnv *, jobject, jlong native_application, int display_rotation, int width, int height);
    JNI_METHOD(void, onGlSurfaceDrawFrame)(JNIEnv *, jclass, jlong native_application);

    JNI_METHOD(void, onTouched)(JNIEnv *, jclass, jlong native_application, jfloat x, jfloat y);
    JNI_METHOD(jboolean, hasDetectedPlanes)(JNIEnv *, jclass, jlong native_application);

    JNIEnv *GetJniEnv();
    jclass FindClass(const char *classname);
}  // extern "C"
#endif
