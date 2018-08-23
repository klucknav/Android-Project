package com.example.kroth.testing;

import android.content.Intent;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity implements GLSurfaceView.Renderer{

    // menber variables //
    private static final String TAG = MainActivity.class.getSimpleName();
    private static final int SNACKBAR_UPDATE_INTERVAL_MILLIS = 1000; // In milliseconds.

    private GLSurfaceView surfaceView;

    private boolean viewportChanged = false;
    private int viewportWidth;
    private int viewportHeight;

    private long nativeApplication;
    private GestureDetector gestureDetector;
    private Snackbar loadingMessageSnackbar;
    private Handler planeStatusCheckingHandler;

    TextView textView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = findViewById(R.id.sample_text);

        getSupportActionBar().setTitle("Testing...");

        setUpRenderer();
        setUpGestureDetector();

        // JNI
        JniInterface.assetManager = getAssets();
        nativeApplication = JniInterface.createNativeApplication(getAssets());
    }

    private void setUpRenderer(){
        surfaceView = findViewById(R.id.surfaceview);
        surfaceView.setPreserveEGLContextOnPause(true);
        surfaceView.setEGLContextClientVersion(2);
        surfaceView.setEGLConfigChooser(8,8,8,8,16,0);
        surfaceView.setRenderer(this);
        surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }
    private void setUpGestureDetector() {
        gestureDetector = new GestureDetector(this, new GestureDetector.SimpleOnGestureListener(){
            @Override
            public boolean onSingleTapUp(MotionEvent e) {
                textView.setText("I've been tapped");
                return true;
            }

            @Override
            public boolean onDoubleTap(MotionEvent e) {
                textView.setText("Double tapped");
                return true;
            }

            @Override
            public void onLongPress(MotionEvent e) {
                textView.setText("Long Press");
                super.onLongPress(e);
            }

            @Override
            public boolean onDown(MotionEvent e) {
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
    private void createSnackbar(){
        loadingMessageSnackbar = Snackbar.make(MainActivity.this.findViewById(android.R.id.content),
                "Searching for surfaces...", Snackbar.LENGTH_INDEFINITE);
        loadingMessageSnackbar.getView().setBackgroundColor(0xbf323232);
        loadingMessageSnackbar.show();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // ARCore requires camera permissions -> ask user for permission if we don't already have it.
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            CameraPermissionHelper.requestCameraPermission(this);
            return;
        }
        JniInterface.onResume(nativeApplication, getApplicationContext(), this);
        surfaceView.onResume();
        createSnackbar();
    }

    @Override
    protected void onPause() {
        super.onPause();
        surfaceView.onPause();
        JniInterface.onPause(nativeApplication);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        synchronized (this){
            JniInterface.destroyNativeApplication(nativeApplication);
            nativeApplication = 0;
        }
    }

    // -------------------- Renderer methods -------------------- //
    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        JniInterface.onGlSurfaceCreated(nativeApplication);
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        viewportWidth = width;
        viewportHeight = height;
        viewportChanged = true;
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        synchronized (this){
            if(nativeApplication == 0){
                return;
            }
            if(viewportChanged){
                int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                JniInterface.onDisplayGeometryChanged(nativeApplication, displayRotation, viewportWidth, viewportHeight);
                viewportChanged = false;
            }
            JniInterface.onGlSurfaceDrawFrame(nativeApplication);
        }
    }
    // -------------------- END Renderer Methods -------------------- //

    public void openOSG_Activity(){
        Intent intent = new Intent(this, OSG_Activity.class);
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
                openOSG_Activity();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
