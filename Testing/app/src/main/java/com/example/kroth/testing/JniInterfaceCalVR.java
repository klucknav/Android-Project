package com.example.kroth.testing;

import android.content.res.AssetManager;

public class JniInterfaceCalVR {

    static {
        System.loadLibrary("simpleCalVRd");
    }

    private static final String TAG = "JniInterfaceCalVR";
    static AssetManager assetManager;

    public static native long JNIcreateController(AssetManager asset_manager);

    public static native void JNIonGlSurfaceCreated(String calvr_path);

    public static native void JNIonResourceLoaded(String path);

    public static native void JNIonViewChanged(int rot, int width, int height);

    public static native void JNIdrawFrame();

    // mouse events
    public static native void JNIpressMouse(boolean down, float x, float y);
    public static native void JNImoveMouse(float x, float y);
    public static native void JNIsetDelta(float deltaX, float deltaY);
    public static native void testing();
    public static native void doubleTap(float x, float y);
    public static native void longPress(float x, float y);
}
