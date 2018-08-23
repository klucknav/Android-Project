#include "jni_interface_calvr.h"
#include "util.h"

namespace {
    //maintain a reference to VM
    static JavaVM *g_vm = nullptr;

    //global environment
    jlong calvrAppPtr = 0;

    inline jlong controllerPtr(calvr_application * native_controller){
        return reinterpret_cast<intptr_t>(native_controller);
    }
    inline calvr_application * controllerNative(jlong ptr){
        return reinterpret_cast<calvr_application *>(ptr);
    }
}

//JNI Library function: call when native lib is load(System.loadLibrary)
jint JNI_OnLoad(JavaVM *vm, void *) {
    g_vm = vm;
    return JNI_VERSION_1_6;
}

JNI_METHOD(jlong, JNIcreateController)(JNIEnv *env, jclass type, jobject asset_manager){

    LOGI("---------- JNI CREATE CONTROLLER ----------");
    AAssetManager *cpp_asset_manager = AAssetManager_fromJava(env, asset_manager);
    LOGI("---------- asset manager set ----------");
    calvr_application *theApp = new calvr_application(cpp_asset_manager);
    LOGI("---------- created a new calvr_app ----------");
    calvrAppPtr = controllerPtr(theApp);
    return calvrAppPtr;
}

JNI_METHOD(void, JNIonGlSurfaceCreated)(JNIEnv *env, jclass type, jstring calvr_path) {

    const char *cpath = env->GetStringUTFChars(calvr_path, JNI_FALSE);

    controllerNative(calvrAppPtr)->onCreate(cpath);

    env->ReleaseStringUTFChars(calvr_path, cpath);
}
JNI_METHOD(void, JNIonResourceLoaded)(JNIEnv *env, jclass type, jstring path){
    const char *cpath = env->GetStringUTFChars(path, JNI_FALSE);

    controllerNative(calvrAppPtr)->onResourceLoaded(cpath);

    env->ReleaseStringUTFChars(path, cpath);
}
JNI_METHOD(void, JNIonViewChanged)(JNIEnv *env, jclass type, jint rot, jint width, jint height){

    controllerNative(calvrAppPtr)->onViewChanged(rot, width, height);

}
JNI_METHOD(void, JNIdrawFrame)(JNIEnv *env, jclass type){

    controllerNative(calvrAppPtr)->onDrawFrame();

}

JNI_METHOD(void, JNIpressMouse)(JNIEnv *env, jclass type, jboolean down, jfloat x, jfloat y) {
    controllerNative(calvrAppPtr)->pressMouse(down, x, y);
}
JNI_METHOD(void, JNImoveMouse)(JNIEnv *env, jclass type, jfloat x, jfloat y){
    controllerNative(calvrAppPtr)->moveMouse(x, y);
}
JNI_METHOD(void, JNIsetDelta)(JNIEnv *env, jclass type, jfloat deltaX, jfloat deltaY){
    controllerNative(calvrAppPtr)->setDelta(deltaX, deltaY);
}
JNI_METHOD(void, testing)(JNIEnv *env, jclass type){
    controllerNative(calvrAppPtr)->testing();
}
JNI_METHOD(void, doubleTap)(JNIEnv *env, jclass type, jfloat x, jfloat y){
    controllerNative(calvrAppPtr)->doubleTap(x, y);
}
JNI_METHOD(void, longPress)(JNIEnv *env, jclass type, jfloat x, jfloat y) {
    controllerNative(calvrAppPtr)->longPress(x, y);
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




