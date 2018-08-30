package com.example.kroth.testing;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;
import android.util.Log;

import java.io.IOException;

public class JniInterfaceOSG {
    static {
        System.loadLibrary("arBasic");
    }

    private static final String TAG = "JniInterfaceOSG";
    static AssetManager assetManager;

    public static long createController(){
        return JNIcreateController(assetManager);
    }
    //pass a Java object
    public static native long JNIcreateController(AssetManager asset_manager);

    public static native void JNIdrawFrame();

    public static native void JNIonGlSurfaceCreated();
    public static native void JNIonViewChanged(int rotation, int width, int height);

    public static native void JNIonTouched(float x, float y);
    public static native void JNIonLongPress(float x, float y);

    public static native void JNImoveMouse(float x, float y);
    public static native void JNIpressMouse(boolean down, float x, float y);
    public static native void JNIsetDelta(float deltaX, float deltaY, float x, float y);
    public static native void JNIsetMode(int newMode);
    public static native void JNIreset();
    public static native void JNIswitchView();

    public static native boolean JNIhasDetectedPlane();

    public static native void JNIonResume(Context context, Activity activity);
    public static native void JNIonPause();
    public static native void JNIonDestroy();
    public static native void JNILoadScene(String filename);

    public static Bitmap loadImage(String imageName) {

        try {
            return BitmapFactory.decodeStream(assetManager.open(imageName));
        } catch (IOException e) {
            Log.e(TAG, "Cannot open image " + imageName);
            return null;
        }
    }

    public static void loadTexture(int target, Bitmap bitmap) {
        GLUtils.texImage2D(target, 0, bitmap, 0);
    }
}
