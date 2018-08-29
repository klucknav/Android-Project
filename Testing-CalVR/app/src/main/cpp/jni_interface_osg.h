#ifndef TESTING_JNI_INTERFACE_OSG_H
#define TESTING_JNI_INTERFACE_OSG_H

#include <jni.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_example_kroth_testing_JniInterfaceOSG_##method_name

extern "C" {
    JNI_METHOD(jlong, JNIcreateController)(JNIEnv *env, jclass type, jobject asset_manager);
    JNI_METHOD(void, JNIdrawFrame)(JNIEnv *env, jclass type);
    JNI_METHOD(void, JNIonGlSurfaceCreated)(JNIEnv *env, jclass type);
    JNI_METHOD(void, JNIonViewChanged)(JNIEnv*, jclass, jint rotation, jint width, jint height);

    JNI_METHOD(void, JNIonTouched)(JNIEnv *, jclass, jfloat x, jfloat y);
    JNI_METHOD(void, JNIonLongPress)(JNIEnv *, jclass, jfloat x, jfloat y);

    JNI_METHOD(void, JNImoveMouse)(JNIEnv *env, jclass type, jfloat x, jfloat y);
    JNI_METHOD(void, JNIpressMouse)(JNIEnv *env, jclass type, jboolean down, jfloat x, jfloat y);
    JNI_METHOD(void, JNIsetDelta)(JNIEnv *env, jclass type, jfloat deltaX, jfloat deltaY, jfloat x, jfloat y);
    JNI_METHOD(void, JNIsetMode)(JNIEnv *env, jclass type, jint newMode);
    JNI_METHOD(void, JNIreset)(JNIEnv *env, jclass type);
    JNI_METHOD(void, JNIswitchView)(JNIEnv *env, jclass type);

    JNI_METHOD(jboolean, JNIhasDetectedPlane)(JNIEnv*, jclass);

    JNI_METHOD(void, JNIonResume)(JNIEnv *, jclass,jobject contex, jobject activitiy);
    JNI_METHOD(void, JNIonPause)(JNIEnv *, jclass);
    JNI_METHOD(void, JNIonDestroy)(JNIEnv *, jclass, long controller_addr);
    JNI_METHOD(void, JNILoadScene)(JNIEnv *, jclass, jstring filename);

    //access JNI env on the current thread
    JNIEnv * GetJniEnv();
    jclass FindClass(const char *classname);
}

#endif //TESTING_JNI_INTERFACE_OSG_H
