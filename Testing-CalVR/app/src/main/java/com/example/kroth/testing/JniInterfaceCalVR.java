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
    public static native void JNImoveMouse(float deltaX, float deltaY, float x, float y);
    public static native void JNIsetDelta(float deltaX, float deltaY);

    public static native void JNIsetMode(int newMode);
    public static native void JNIreset();

    // to open and close the menu
    public static native void doubleTap(float x, float y);
    public static native void longPress(float x, float y);

    // to switch between moving the OSG object and the CalVR screen
    public static native void JNIoneFingerDoubleTap();
}
