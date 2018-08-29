#ifndef TESTING_CALVR_APPLICATION_H
#define TESTING_CALVR_APPLICATION_H

// Android
#include <android/asset_manager.h>
// OSG
#include <osgViewer/Viewer>
// CalVR
#include <cvrKernel/CVRViewer.h>

// Other
#include "SpatialViz.h"
#include "../../../../../../calvr_plugins/general/MenuBasics/MenuBasics.h"

enum{
    MOVE_WORLD, // 0
    SCALE,      // 1
    DRIVE,      // 2
    FLY         // 3
};

class calvr_application{
private:
    AAssetManager *const _asset_manager;
    osg::ref_ptr<osg::Group>  _root;

    // CalVR
    cvr::CVRViewer * _viewer;
    cvr::SceneManager * _scene;
    cvr::ConfigManager * _config;
    cvr::MenuManager * _menuManager;
    cvr::InteractionManager *_interactionManager;   // for touch events
    cvr::ComController *_comController;
    cvr::TrackingManager *_trackingManager;
    cvr::Navigation *_navigation;

    MenuBasics* _menuBasics;
    SpatialViz* _spatialViz;

    // ADDED for translating and rotating the osg object
    osg::ref_ptr<osg::PositionAttitudeTransform> _sphereTrans;
    osg::ref_ptr<osg::PositionAttitudeTransform> _cubeTrans;
    osg::ref_ptr<osg::PositionAttitudeTransform> _currTrans;
    osg::Vec3 lightDir = osg::Vec3(0,0,5);
    osg::Vec3 xAxis = osg::Vec3(1.0, 0.0, 0.0);
    osg::Vec3 yAxis = osg::Vec3(0.0, 1.0, 0.0);
    osg::Vec3 zAxis = osg::Vec3(0.0, 0.0, 1.0);
    int currMode = DRIVE;
    bool moveOBJ = true;

    // to reset position and orientation
    osg::Vec3 defaultScale = osg::Vec3(1,1,1);
    osg::Vec3 defaultPos = osg::Vec3(0,0,0);
    osg::Quat defaultRot = osg::Quat(0, xAxis, 0, yAxis, 0, zAxis);
    osg::Quat totalRotation = osg::Quat(0, xAxis, 0, yAxis, 0, zAxis);

    // to go between android coordinates to CalVR and OSG coordinates
    int _screenWidth, _screenHeight;
    double ratio, objRatio;

    void initialize_camera();
    void createDebugOSGsphere(osg::Vec3 pos);
    void createDebugOSGcube(osg::Vec3 pos);
    void setupDefaultEnvironment(const char* root_path);

public:
    calvr_application(AAssetManager *assetManager);

    void onCreate(const char * calvr_path);
    void onDrawFrame();
    void onViewChanged(int rotation, int width, int height);

    void onResourceLoaded(const char* path);

    void pressMouse(bool down, float x, float y);
    void moveMouse(float delta_x, float delta_y, float x, float y);
    void setDelta(float delta_x, float delta_y);

    // to open and close the menu
    void doubleTap(float x, float y);
    void longPress(float x, float y);

    // move modes
    void switchMoveMode();      // switch between moving the object and CalVR controls
    void setMode(int newMode);  // how to move the obj
    void reset();
    osg::Vec3f screenToWorld(float x, float y);
    osg::Vec3 screenToWorldObj(float x, float y);
};


#endif //TESTING_CALVR_APPLICATION_H
