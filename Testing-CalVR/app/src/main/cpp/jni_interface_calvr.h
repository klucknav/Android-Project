#ifndef TESTING_JNI_INTERFACE_CALVR_H
#define TESTING_JNI_INTERFACE_CALVR_H

// Android
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>

// CalVR
#include "calvr_application.h"

#define JNI_METHOD(returnType, funcName)\
        JNIEXPORT returnType JNICALL    \
            Java_com_example_kroth_testing_JniInterfaceCalVR_##funcName

extern "C"{

    JNI_METHOD(jlong, JNIcreateController)(JNIEnv *env, jclass type, jobject asset_manager);
    JNI_METHOD(void, JNIonGlSurfaceCreated)(JNIEnv *env, jclass type, jstring calvr_path);

    JNI_METHOD(void, JNIonResourceLoaded)(JNIEnv *env, jclass type, jstring path);
    JNI_METHOD(void, JNIonViewChanged)(JNIEnv *env, jclass type, jint rot, jint width, jint height);

    JNI_METHOD(void, JNIdrawFrame)(JNIEnv *env, jclass type);

    // Mouse events
    JNI_METHOD(void, JNIpressMouse)(JNIEnv *env, jclass type, jboolean down, jfloat x, jfloat y);
    JNI_METHOD(void, JNImoveMouse)(JNIEnv *env, jclass type, jfloat deltaX, jfloat deltaY, jfloat x, jfloat y);
    JNI_METHOD(void, JNIsetDelta)(JNIEnv *env, jclass type, jfloat deltaX, jfloat deltaY);

    JNI_METHOD(void, JNIsetMode)(JNIEnv *env, jclass type, jint newMode);
    JNI_METHOD(void, JNIreset)(JNIEnv *env, jclass type);

    // to open and close the menu
    JNI_METHOD(void, doubleTap)(JNIEnv *env, jclass type, jfloat x, jfloat y);
    JNI_METHOD(void, longPress)(JNIEnv *env, jclass type, jfloat x, jfloat y);

    // to switch between moving the OSG object and the CalVR screen
    JNI_METHOD(void, JNIoneFingerDoubleTap)(JNIEnv *env, jclass type);

    //access JNI env on the current thread
    JNIEnv * GetJniEnv();
    jclass FindClass(const char *classname);
}




#endif //TESTING_JNI_INTERFACE_CALVR_H
