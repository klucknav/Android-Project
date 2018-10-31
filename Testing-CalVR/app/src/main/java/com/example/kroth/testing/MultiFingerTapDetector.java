package com.example.kroth.testing;

import android.util.Log;
import android.view.MotionEvent;
import android.view.ViewConfiguration;

public abstract class MultiFingerTapDetector {

    private static final int TIMEOUT = ViewConfiguration.getDoubleTapTimeout() + 100;
    private static final int TRIPLE_TIMEOUT = ViewConfiguration.getDoubleTapTimeout() + 250;
    private static final int LONG_PRESS_TIMEOUT = ViewConfiguration.getLongPressTimeout();

    // for one finger taps
    private long mFirstDownTimeOne = 0;
    private boolean mSeparateTouchesOne = false;
    private byte mNumOneFingerTaps = 0;

    // for two finger taps
    private long mFirstDownTimeTwo = 0;
    private boolean mSeparateTouchesTwo = false;
    private byte mNumTwoFingerTaps = 0;
    MotionEvent secondaryFingerEvent;
    private float x1, x2, y1, y2, avgX, avgY;

    // for three finger taps
    private long mFirstDownTimeThree = 0;
    private boolean mSeparateTouchesThree = false;
    private byte mNumThreeFingerTaps = 0;

    private boolean oneFingerDown = false;
    private boolean twoFingerDown = false;

    private void resetOne(long time){
        mFirstDownTimeOne = time;
        mSeparateTouchesOne = false;
        mNumOneFingerTaps = 0;
    }

    private void resetTwo(long time) {
        mFirstDownTimeTwo = time;
        mSeparateTouchesTwo = false;
        mNumTwoFingerTaps = 0;
    }

    private void resesetThree(long time){
        mFirstDownTimeThree = time;
        mSeparateTouchesThree = false;
        mNumThreeFingerTaps = 0;
    }

    public boolean onTouchEvent(MotionEvent event) {
        switch(event.getActionMasked()) {

            // ACTION_DOWN - when the first finger touches the screen
            case MotionEvent.ACTION_DOWN:

                if(event.getPointerCount() == 1){
                    onOneFingerDown(event);
                    oneFingerDown = true;
                }
                // if this is the first tap OR we are tapping too slow -> resetTwo
                if(mFirstDownTimeOne == 0 || event.getEventTime() - mFirstDownTimeOne > TIMEOUT) {
                    resetOne(event.getDownTime());
                }

                // if this is the first tap OR we are tapping too slow -> resetTwo
                if(mFirstDownTimeTwo == 0 || event.getEventTime() - mFirstDownTimeTwo > TIMEOUT)
                    resetTwo(event.getDownTime());

                // if this is the first tap OR we are tapping too slow -> resetTwo the three touch variables
                if(mFirstDownTimeThree == 0 || event.getEventTime() - mFirstDownTimeThree > TRIPLE_TIMEOUT)
                    resesetThree(event.getDownTime());
                break;

            // as soon as the second finger touches the screen no longer a oneTouch event
            case MotionEvent.ACTION_POINTER_DOWN:
                oneFingerDown = false;
                onMoreFingersDown(event);
                if (event.getPointerCount() == 2)
                    twoFingerDown = true;
                break;

            // POINTER_UP - when a second(+) finger leaves the screen
            case MotionEvent.ACTION_POINTER_UP:

                // TESTING - setting secondary event here doesn't work (same as primary)

                // if we have two fingers touching the screen -> increment the number of two finger taps
                if(event.getPointerCount() == 2) {
                    mNumTwoFingerTaps++;
                    mFirstDownTimeOne = 0;
                    secondaryFingerEvent = event;       // TESTING - same problem
                    x1 = event.getX(0);     y1 = event.getY(0);
                    x2 = event.getX(1);     y2 = event.getY(1);
                    avgX = (x1+x2)/2;
                    avgY = (y1+y2)/2;
                    Log.i("DETECTOR", "x1 = " + x1 + " y1 = " + y1 + " x2 = " + x2 + " y2 = " + y2);
                }
                // if we have three fingers touching the screen -> increment the number of three finger taps
                if(event.getPointerCount() == 3) {
                    mNumThreeFingerTaps++;
                    mFirstDownTimeTwo = 0;
                }

                // if we pick up a 4th... finger then resetTwo the time of the first tap
                // because it is no longer a double tap
                else if (event.getPointerCount() > 3){
                    mFirstDownTimeOne = 0;
                    mFirstDownTimeTwo = 0;
                    mFirstDownTimeThree = 0;
                }
                break;

            // UP - when the first finger leaves the screen
            case MotionEvent.ACTION_UP:

                if(oneFingerDown){
                    onOneFingerUp(event);
                    oneFingerDown = false;
                }
                if (event.getPointerCount() == 1) {
                    mNumOneFingerTaps++;
                }

                // Once the first finger is removed, we have ended the first tap
                if(!mSeparateTouchesOne){
                    mSeparateTouchesOne = true;
                }
                if(!mSeparateTouchesTwo) {
                    mSeparateTouchesTwo = true;
                    // if we had two fingers touching the screen 
                    if (twoFingerDown){
                        onTwoFingersUp(avgX, avgY);
                        twoFingerDown = false;
                    }
                }
                if(!mSeparateTouchesThree){
                    mSeparateTouchesThree = true;
                }
                if (mSeparateTouchesOne) {
                    if (mNumOneFingerTaps == 2 && event.getEventTime() - mFirstDownTimeOne < TIMEOUT) {
                        onOneFingerDoubleTap(event.getX(), event.getY());
                        return true;
                    }
                    if (mNumOneFingerTaps == 3 && event.getEventTime() - mFirstDownTimeOne < TRIPLE_TIMEOUT){
                        onOneFingerTripleTap();
                        mFirstDownTimeOne = 0;
                        return true;
                    }
                }
                // when the additional finger leaves the screen, if we had two fingers and the
                // time between downs was below the threshold -> double tap
                if (mSeparateTouchesTwo && !oneFingerDown) {
                    if (mNumTwoFingerTaps == 2 && event.getEventTime() - mFirstDownTimeTwo < TIMEOUT) {
                        onTwoFingerDoubleTap(avgX, avgY);
                        return true;
                    }
                    if (mNumTwoFingerTaps == 3 && event.getEventTime() - mFirstDownTimeTwo < TRIPLE_TIMEOUT){
                        onTwoFingerTripleTap(avgX, avgY);
                        mFirstDownTimeTwo = 0;
                        return true;
                    }
                }
                if (mSeparateTouchesThree) {
                    if (mNumThreeFingerTaps == 2 && event.getEventTime() - mFirstDownTimeThree < TIMEOUT){
                        onThreeFingerDoubleTap();
                    }
                    if (mNumThreeFingerTaps == 3 && event.getEventTime() - mFirstDownTimeThree < TRIPLE_TIMEOUT){
                        onThreeFingerTripleTap();
                        mFirstDownTimeThree = 0;
                    }
                    return true;
                }

            // MOVE when the finger moves at all
            case MotionEvent.ACTION_MOVE:

                // if we have just the one finger touching the screen 
                if (event.getPointerCount() == 1 && oneFingerDown){
                    onOneFingerMove(event);
                    return true;
                }
                /*// if we have just the one finger touching the screen and time is longer than the TIMEOUT
                if (event.getPointerCount() == 1 && oneFingerDown && event.getEventTime() - event.getDownTime() > LONG_PRESS_TIMEOUT){
                    onOneFingerLongPress(event);
                    return true;
                }*/
                // if we have two fingers down and time is longer than the TIMEOUT
                if (event.getPointerCount() == 2 && event.getEventTime() - event.getDownTime() > LONG_PRESS_TIMEOUT) {
                    onTwoFingerLongPress(event);
                    return true;
                }

                // if we have two fingers down and time is longer than the TIMEOUT
                if (event.getPointerCount() == 3 && event.getEventTime() - event.getDownTime() > LONG_PRESS_TIMEOUT) {
                    onThreeFingerLongPress(event);
                    return true;
                }
                if (event.getPointerCount() == 2 && !oneFingerDown){
                    onTwoFingersMove(event);
                    return true;
                }
                if (event.getPointerCount() == 3 && !oneFingerDown){
                    onThreeFingersMove(event);
                    return true;
                }
        }

        return false;
    }


    // Methods that need to be overridden
    //public abstract void testing();
    public abstract void onOneFingerDown(MotionEvent event);
    public abstract void onOneFingerMove(MotionEvent event);
    public abstract void onOneFingerUp(MotionEvent event);

    public abstract void onMoreFingersDown(MotionEvent event);
    public abstract void onTwoFingersMove(MotionEvent event);
    public abstract void onThreeFingersMove(MotionEvent event);

    public abstract void onOneFingerDoubleTap(float x, float y);
    public abstract void onOneFingerTripleTap();
    public abstract void onOneFingerLongPress(MotionEvent event);

    // TWO FINGER TAPS
    public abstract void onTwoFingersUp(float avgX, float avgY);
    public abstract void onTwoFingerDoubleTap(float avgX, float avgY);
    public abstract void onTwoFingerTripleTap(float avgX, float avgY);
    public abstract void onTwoFingerLongPress(MotionEvent event);

    // THREE FINGER TAPS
    public abstract void onThreeFingerDoubleTap();
    public abstract void onThreeFingerTripleTap();
    public abstract void onThreeFingerLongPress(MotionEvent event);

}
