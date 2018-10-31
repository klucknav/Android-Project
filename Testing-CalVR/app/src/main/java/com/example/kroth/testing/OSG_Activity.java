package com.example.kroth.testing;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.hardware.display.DisplayManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.DisplayMetrics;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.Objects;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class OSG_Activity extends AppCompatActivity implements DisplayManager.DisplayListener{

    private GLSurfaceView surfaceView;

    private long appPtr;
    private boolean viewportChanged = false;
    private int viewportWidth;
    private int viewportHeight;

    private GestureDetector gestureDetector;
    private MultiFingerTapDetector multiFingerTapDetector;
    private TextView message;

    // member variables for movement
    private float mPrevX, mPrevY;
    private float mDensity;
    // for the view modes
    private int MOVE_WORLD = 0;
    private int SCALE = 1;
    private int DRIVE = 2;
    private int FLY = 3;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_osg);
        message = findViewById(R.id.textView);

        JniInterfaceOSG.assetManager = getAssets();
        appPtr = JniInterfaceOSG.createController();

        Objects.requireNonNull(getSupportActionBar()).setTitle("Using OSG to render");

        // for touch events
        final DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        mDensity = displayMetrics.density;


        setupSurfaceView();
        //setUpGestureDetector();
        setUpMultiGesture();
        //setupResources("/box.osgt", R.raw.box);
    }

    private void setupSurfaceView() {
        surfaceView = (GLSurfaceView) findViewById(R.id.surfaceview);
        // Set up renderer.
        surfaceView.setPreserveEGLContextOnPause(true);
        surfaceView.setEGLContextClientVersion(2);
        surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        surfaceView.setRenderer(new OSG_Activity.Renderer());
        surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }
    @SuppressLint("ClickableViewAccessibility")


    private void setUpGestureDetector(){
        gestureDetector = new GestureDetector(this, new GestureDetector.SimpleOnGestureListener(){
            @Override
            public boolean onSingleTapUp(MotionEvent e) {
                float x = e.getX();
                float y = e.getY();
                message.setText("Single tap");
                JniInterfaceOSG.JNIpressMouse(false, x, y);
                return true;
            }

            @Override
            public boolean onDoubleTap(MotionEvent e) {
                message.setText("Double tapped");
                return true;
            }

            @Override
            public void onLongPress(MotionEvent e) {
                message.setText("Long Press");
//                float x = e.getX();
//                float y = e.getY();
//                float x2 = (float) (x/525.0 - 1.0);
//                float y2 = (float) -(y/1000.0 - 1.0);
//                JniInterfaceOSG.JNIonLongPress(x2, y2);
                super.onLongPress(e);
            }

            @Override
            public boolean onDown(MotionEvent e) {
                message.setText("pressed down");
                JniInterfaceOSG.JNIpressMouse(true, e.getX(), e.getY());
                return true;
            }
        });

        surfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
            return gestureDetector.onTouchEvent(motionEvent);
            }
        });
    }
    private void setUpMultiGesture(){
        multiFingerTapDetector = new MultiFingerTapDetector() {
            @Override
            public void onOneFingerDown(MotionEvent event) {
                message.setText("1 finger - DOWN");
                mPrevX = event.getX();          mPrevY = event.getY();
                //JniInterfaceOSG.JNIpressMouse(true, mPrevX, mPrevY);
            }

            @Override
            public void onOneFingerMove(MotionEvent event) {
                message.setText("1 finger - MOVE");
                float x = event.getX();
                float y = event.getY();

                // for rotation and scale
                float deltaX = (x - mPrevX) / mDensity / 50f;
                float deltaY = (y - mPrevY) / mDensity / 50f;

                JniInterfaceOSG.JNIsetDelta(deltaX, deltaY, x, y);
                mPrevX = x;
                mPrevY = y;
            }

            @Override
            public void onOneFingerUp(MotionEvent event) {
                message.setText("1 finger - Tap");
                //JniInterfaceOSG.JNIpressMouse(false, event.getX(), event.getY());
            }

            @Override
            public void onMoreFingersDown(MotionEvent event) {
                message.setText("2+ fingers - DOWN");
            }

            @Override
            public void onTwoFingersMove(MotionEvent event) {
                message.setText("2 fingers - MOVE");
            }

            @Override
            public void onThreeFingersMove(MotionEvent event) {
                message.setText("3 fingers - MOVE");
            }

            @Override
            public void onOneFingerDoubleTap(float x, float y) {
                message.setText("1 finger - Double Tap");
                JniInterfaceOSG.JNIswitchView();
            }

            @Override
            public void onOneFingerTripleTap() {
                message.setText("1 finger - Tripple Tap");
            }

            @Override
            public void onOneFingerLongPress(MotionEvent event) {
                message.setText("1 finger - Long Press");
            }

            @Override
            public void onTwoFingersUp(float avgX, float avgY){
                message.setText("2 finger - Single Tap");
            }

            @Override
            public void onTwoFingerDoubleTap(float avgX, float avgY) {
                message.setText("2 finger - Double Tap");
            }

            @Override
            public void onTwoFingerTripleTap(float avgX, float avgY) {
                message.setText("2 finger - Triple Tap");
            }

            @Override
            public void onTwoFingerLongPress(MotionEvent event) {
                message.setText("2 finger - Long Press");
            }

            @Override
            public void onThreeFingerDoubleTap() {
                message.setText("3 finger - Double Tap");
            }

            @Override
            public void onThreeFingerTripleTap() {
                message.setText("3 finger - Triple Tap");
            }

            @Override
            public void onThreeFingerLongPress(MotionEvent event) {
                message.setText("3 finger - Long Press");
            }
        };

        surfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                multiFingerTapDetector.onTouchEvent(motionEvent);
                return true;
            }
        });
    }

    private void setupResources(String filename, int id) {
        // Get absolute path to internal storage.
        File dir = getFilesDir();
        String path = dir.getAbsolutePath();
        message.setText(path);
        // Check if 'box.osgt' already exists.
        String modelPath = path + filename;
        File model = new File(modelPath);
        // Copy 'box.ogst' from 'res/raw', if it does not exist.
        try {
            if (!model.exists()) {
                // Note: this only works for small files,
                // because we read the whole file into memory.
                InputStream is = getResources().openRawResource(id);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                FileOutputStream os = new FileOutputStream(model);
                os.write(buffer);
            }
            JniInterfaceOSG.JNILoadScene(modelPath);
        }
        catch (Exception e) {

        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        //Request for camera permission, which may be used in ARCore
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            CameraPermissionHelper.requestCameraPermission(this);
            return;
        }
        JniInterfaceOSG.JNIonResume(getApplicationContext(), this);
        surfaceView.onResume();

        Objects.requireNonNull(getSystemService(DisplayManager.class)).registerDisplayListener(this, null);
    }
    @Override
    protected void onPause() {
        super.onPause();
        JniInterfaceOSG.JNIonPause();
        surfaceView.onPause();

        Objects.requireNonNull(getSystemService(DisplayManager.class)).unregisterDisplayListener(this);
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        synchronized (this){
            JniInterfaceOSG.JNIonDestroy();
            appPtr = 0;
        }
    }

    private class Renderer implements GLSurfaceView.Renderer{
        public void onDrawFrame(GL10 gl) {
            // Synchronized to avoid racing onDestroy.
            synchronized (this) {
                if (appPtr == 0)
                    return;
                if (viewportChanged) {
                    int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                    JniInterfaceOSG.JNIonViewChanged(displayRotation, viewportWidth, viewportHeight);
                    viewportChanged = false;
                }
                JniInterfaceOSG.JNIdrawFrame();
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            viewportWidth = width;
            viewportHeight = height;
            viewportChanged = true;
            //JniInterfaceOSG.JNIonViewChanged(0, width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing
            gl.glEnable(GL10.GL_DEPTH_TEST);
            JniInterfaceOSG.JNIonGlSurfaceCreated();
        }
    }

    // -------------------- DISPLAY LISTENER METHODS -------------------- //
    @Override
    public void onDisplayAdded(int i) {    }

    @Override
    public void onDisplayRemoved(int i) {    }

    @Override
    public void onDisplayChanged(int i) { viewportChanged = true; }
    // -------------------- END DISPLAY LISTENER METHODS -------------------- //

    // -------------------- MENU OPTIONS + TO OPEN THE CalVR ACTIVITY -------------------- //
    public void openCalVR_Activity(){
        Intent intent = new Intent(this, CalVR_Activity.class);
        startActivity(intent);
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return true;
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case R.id.activity_item:
                openCalVR_Activity();
                return true;

            case R.id.moveWorld:
                JniInterfaceOSG.JNIsetMode(MOVE_WORLD);
                return true;
            case R.id.scale:
                JniInterfaceOSG.JNIsetMode(SCALE);
                return true;
            case R.id.drive:
                JniInterfaceOSG.JNIsetMode(DRIVE);
                return true;
            case R.id.fly:
                JniInterfaceOSG.JNIsetMode(FLY);
                return true;
            case R.id.reset:
                JniInterfaceOSG.JNIreset();
                return true;


            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
