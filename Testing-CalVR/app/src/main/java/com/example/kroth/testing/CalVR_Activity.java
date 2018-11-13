package com.example.kroth.testing;

import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import java.io.File;
import java.util.Objects;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class CalVR_Activity extends AppCompatActivity {

    final static String TAG = "CalVRActivity";

    private long calVRapp_ptr;
    private GLSurfaceView surfaceView;
    final static private String calvr_folder = "calvrAssets";
    String calvr_dest = null;

    // gesture detection
    private TextView message, sub_message;
    private MultiFingerTapDetector multiFingerTapDetector;
    // member variables for movement
    private float mPrevX, mPrevY;
    private float mInitX, mInitY;
    private float mDensity;
    // for the view modes
    private int MOVE_WORLD = 0;
    private int SCALE = 1;
    private int DRIVE = 2;
    private int FLY = 3;
    private int currMode = MOVE_WORLD;
    private MotionEvent test2Event;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cal_vr);
        message = findViewById(R.id.message);
        sub_message = findViewById(R.id.sub_message);

        Objects.requireNonNull(getSupportActionBar()).setTitle("Adding CalVR");
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);

        // for touch events
        final DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        mDensity = displayMetrics.density;

        JniInterfaceCalVR.assetManager = getAssets();
        copyFromAssets();
        calVRapp_ptr = JniInterfaceCalVR.JNIcreateController(JniInterfaceCalVR.assetManager);

        setupSurfaceView();
        setUpMultiGesture();
    }

    private void setupSurfaceView() {
        surfaceView = (GLSurfaceView) findViewById(R.id.surfaceview);
        // Set up renderer.
        surfaceView.setPreserveEGLContextOnPause(true);
        surfaceView.setEGLContextClientVersion(2);
        surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        surfaceView.setRenderer(new CalVR_Activity.Renderer());
        surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        Log.i(TAG, "surfaceView complete set up");
    }
    private void copyFromAssets(){
        calvr_dest = getFilesDir().getAbsolutePath() + "/" + calvr_folder;
        //Skip copying if files exist
        File destDir = new File(calvr_dest);
        if(destDir.exists()) {
            Log.i(TAG, "directory already exists");
            return;
        }
        try{
            fileUtils.copyFromAsset(getAssets(), calvr_folder, calvr_dest);
        }catch (Exception e){
            Log.e(TAG, "copyFromAssets: Failed to copy from asset folder");
        }
    }
    private void setUpMultiGesture(){
        multiFingerTapDetector = new MultiFingerTapDetector() {
            @Override
            public void onOneFingerDown(MotionEvent event) {
                message.setText("1 finger - DOWN");
                mPrevX = event.getX();          mPrevY = event.getY();
                mInitX = event.getX();          mInitY = event.getY();
                sub_message.setText("");
                JniInterfaceCalVR.JNIpressMouse(true, mPrevX, mPrevY);
                sub_message.setText("called JNIpressMouse");
            }

            @Override
            public void onOneFingerMove(MotionEvent event) {
                message.setText("1 finger - MOVE");
                float x = event.getX();
                float y = event.getY();

                // for rotation and scale
                float deltaX = (x - mPrevX) / mDensity / 50f;
                float deltaY = (y - mPrevY) / mDensity / 50f;

                Log.i(TAG, "x = " + x + " y = " + y);
                Log.i(TAG, "Dx = " + deltaX + " Dy = " + deltaY);
                JniInterfaceCalVR.JNImoveMouse(deltaX, deltaY, x, y);
                sub_message.setText("called JNImoveMouse");
                mPrevX = x;
                mPrevY = y;
            }

            @Override
            public void onOneFingerUp(MotionEvent event) {
                message.setText("1 finger - Tap");
                JniInterfaceCalVR.JNIpressMouse(false, event.getX(), event.getY());
                sub_message.setText("called JNIpressMouse");
            }

            @Override
            public void onMoreFingersDown(MotionEvent event) {
                message.setText("2+ fingers - DOWN");
                test2Event = event;
                sub_message.setText("");
            }

            @Override
            public void onTwoFingersMove(MotionEvent event) {
                message.setText("2 fingers - MOVE");
                sub_message.setText("");
            }

            @Override
            public void onThreeFingersMove(MotionEvent event) {
                message.setText("3 fingers - MOVE");
                sub_message.setText("");
            }

            @Override
            public void onOneFingerDoubleTap(float x, float y) {
                message.setText("1 finger - Double Tap");
                //JniInterfaceCalVR.JNIoneFingerDoubleTap(x, y);
                sub_message.setText("");
            }

            @Override
            public void onOneFingerTripleTap() {
                message.setText("1 finger - Tripple Tap");
                sub_message.setText("");
            }

            @Override
            public void onOneFingerLongPress(MotionEvent event) {
                message.setText("1 finger - Long Press");
                //sub_message.setText("sending event");
                //JniInterfaceCalVR.JNIoneFingerDoubleTap(event.getX(), event.getY());
            }

            @Override
            public void onTwoFingersUp(float avgX, float avgY){
                message.setText("2 finger - Single Tap");
                sub_message.setText("sending event");
                JniInterfaceCalVR.JNItwoTripleTap(avgX, avgY);
                //JniInterfaceCalVR.JNIFingerDoubleTap(avgX, avgY);
            }

            @Override
            public void onTwoFingerDoubleTap(float avgX, float avgY) {
                message.setText("2 finger - Double Tap");
                JniInterfaceCalVR.doubleTap(avgX, avgY);
//                sub_message.setText("1event: " + event.getPointerCount() + "\n2event: " + test2Event.getPointerCount());
            }

            @Override
            public void onTwoFingerTripleTap(float avgX, float avgY) {
                message.setText("2 finger - Triple Tap");
                JniInterfaceCalVR.JNItwoTripleTap(avgX, avgY);
                sub_message.setText("");
            }

            @Override
            public void onTwoFingerLongPress(MotionEvent event) {
                message.setText("2 finger - Long Press");
                float avgX = (event.getX() + event.getX(1))/2;
                float avgY = (event.getY() + event.getY(1))/2;
                JniInterfaceCalVR.longPress(avgX, avgY);
            }

            @Override
            public void onThreeFingerDoubleTap() {
                message.setText("3 finger - Double Tap");
                sub_message.setText("");
            }

            @Override
            public void onThreeFingerTripleTap() {
                message.setText("3 finger - Triple Tap");
                sub_message.setText("");
            }

            @Override
            public void onThreeFingerLongPress(MotionEvent event) {
                message.setText("3 finger - Long Press");
                sub_message.setText("");
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

    private class Renderer implements GLSurfaceView.Renderer {
        private boolean viewportChanged = false;
        private int viewportWidth;
        private int viewportHeight;

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            GLES30.glClearColor(1.0f, .0f, .0f, 1.0f);
            // JNI onSurfaceCreated call
            JniInterfaceCalVR.JNIonGlSurfaceCreated(calvr_dest);
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            viewportWidth = width;
            viewportHeight = height;
            viewportChanged = true;
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            // Synchronized to avoid racing onDestroy.
            synchronized (this) {
                if (calVRapp_ptr == 0) {
                    return;
                }
                if (viewportChanged) {
                    int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                    // JNI on Changed call
                    JniInterfaceCalVR.JNIonViewChanged(displayRotation, viewportWidth, viewportHeight);
                    viewportChanged = false;
                }
                // JNI DrawFrame call
                JniInterfaceCalVR.JNIdrawFrame();
            }
        }
    }

    // -------------------- MENU OPTIONS -------------------- //
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.calvr_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.moveWorld:
                JniInterfaceCalVR.JNIsetMode(MOVE_WORLD);
                return true;
            case R.id.scale:
                JniInterfaceCalVR.JNIsetMode(SCALE);
                return true;
            case R.id.drive:
                JniInterfaceCalVR.JNIsetMode(DRIVE);
                return true;
            case R.id.fly:
                JniInterfaceCalVR.JNIsetMode(FLY);
                return true;
            case R.id.reset:
                JniInterfaceCalVR.JNIreset();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    // ------------------ END MENU OPTIONS ------------------- //
}
