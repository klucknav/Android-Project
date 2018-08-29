#include <string>
#include <GLES2/gl2.h>
#include <android/asset_manager_jni.h>

#include "jni_interface_osg.h"
#include "osg_application.h"

using namespace std;

namespace {
    // maintain a reference to the JVM
    static JavaVM *g_vm = nullptr;
    jlong nativePtr = 0;

    inline jlong jptr(osgApp::osgApplication *native_ar_application) {
        return reinterpret_cast<intptr_t>(native_ar_application);
    }

    inline osgApp::osgApplication *native(jlong ptr) {
        return reinterpret_cast<osgApp::osgApplication *>(ptr);
    } // namespace
}
//JNI Library function: call when native lib is load(System.loadLibrary)
jint JNI_OnLoad(JavaVM *vm, void *) {
    g_vm = vm;
    return JNI_VERSION_1_6;
}

JNI_METHOD(jlong, JNIcreateController)(JNIEnv* env, jclass type, jobject asset_manager){
    AAssetManager *cpp_asset_manager = AAssetManager_fromJava(env, asset_manager);
    nativePtr = jptr(new osgApp::osgApplication(cpp_asset_manager));
    return nativePtr;
}

JNI_METHOD(void, JNIdrawFrame)(JNIEnv *env, jclass type){
    native(nativePtr)->onDrawFrame();
}
JNI_METHOD(void, JNIonGlSurfaceCreated)(JNIEnv *env, jclass type){
    native(nativePtr)->onCreate();
}
JNI_METHOD(void, JNIonViewChanged)(JNIEnv*, jclass, jint rotation, jint width, jint height){
    native(nativePtr)->onViewChanged(rotation, width, height);
}


JNI_METHOD(void, JNIonTouched)(JNIEnv*, jclass, jfloat x, jfloat y){
    native(nativePtr)->onTouched(x, y);
}
JNI_METHOD(void, JNIonLongPress)(JNIEnv *, jclass, jfloat x, jfloat y){
    native(nativePtr)->onLongPress(x, y);
}
JNI_METHOD(void, JNImoveMouse)(JNIEnv *env, jclass type, jfloat x, jfloat y) {
    native(nativePtr)->moveMouse(x, y);
}
JNI_METHOD(void, JNIpressMouse)(JNIEnv *env, jclass type, jboolean down, jfloat x, jfloat y) {
    native(nativePtr)->pressMouse(down, x, y);
}
JNI_METHOD(void, JNIsetDelta)(JNIEnv *env, jclass type, jfloat deltaX, jfloat deltaY, jfloat x, jfloat y) {
    native(nativePtr)->setDelta(deltaX, deltaY, x, y);
}
JNI_METHOD(void, JNIsetMode)(JNIEnv *env, jclass type, jint newMode) {
    native(nativePtr)->setMode(newMode);
}
JNI_METHOD(void, JNIreset)(JNIEnv *env, jclass type) {
    native(nativePtr)->reset();
}
JNI_METHOD(void, JNIswitchView)(JNIEnv *env, jclass type){
    native(nativePtr)->switchView();
}


JNI_METHOD(jboolean, JNIhasDetectedPlane)(JNIEnv*, jclass){
    return static_cast<jboolean> (native(nativePtr)->hasDetectedPlane() ? JNI_TRUE : JNI_FALSE);
}


JNI_METHOD(void, JNIonResume)(JNIEnv *env, jclass,jobject contex, jobject activitiy){
    native(nativePtr)->onResume(env, contex, activitiy);
}
JNI_METHOD(void, JNIonPause)(JNIEnv *, jclass){
    native(nativePtr)->onPause();
}
JNI_METHOD(void, JNIonDestroy)(JNIEnv *, jclass, long controller_addr){
    delete native(nativePtr);
}
JNI_METHOD(void, JNILoadScene)(JNIEnv *env, jclass, jstring filename){
    const char* cpath = env->GetStringUTFChars(filename, JNI_FALSE);
    native(nativePtr)->loadScene(cpath);
    env->ReleaseStringUTFChars(filename, cpath);
}

JNIEnv *GetJniEnv() {
    JNIEnv *env;
    jint result = g_vm->AttachCurrentThread(&env, nullptr);
    return result == JNI_OK ? env : nullptr;
}

jclass FindClass(const char *classname) {
    JNIEnv *env = GetJniEnv();
    return env->FindClass(classname);
}

