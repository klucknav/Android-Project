#ifndef TESTING_CALVR_APPLICATION_H
#define TESTING_CALVR_APPLICATION_H

// Android
#include <android/asset_manager.h>
// OSG
#include <osgViewer/Viewer>
// CalVR
#include <cvrKernel/CVRViewer.h>

#include "SpatialViz.h"
#include "../../../../../../calvr_plugins/general/MenuBasics/MenuBasics.h"

class calvr_application{
private:
    AAssetManager *const _asset_manager;
    //osgViewer::Viewer * _viewer;
    cvr::CVRViewer * _viewer;
    osg::ref_ptr<osg::Group>  _root;

    cvr::SceneManager * _scene;
    cvr::ConfigManager * _config;
    cvr::MenuManager * _menuManager;
    cvr::InteractionManager *_interactionManager;   // ADDED

    MenuBasics* _menuBasics;
    SpatialViz* _spatialViz;

    // ADDED
    osg::ref_ptr<osg::PositionAttitudeTransform> _sphereTrans;
    osg::ref_ptr<osg::PositionAttitudeTransform> _cubeTrans;
    osg::Vec3 lightDir = osg::Vec3(0,0,5);
    osg::Vec3 xAxis = osg::Vec3(1.0, 0.0, 0.0);
    osg::Vec3 yAxis = osg::Vec3(0.0, 1.0, 0.0);
    osg::Vec3 zAxis = osg::Vec3(0.0, 0.0, 1.0);
    osg::Quat totalRotation = osg::Quat(0, xAxis, 0, yAxis, 0, zAxis);
    float currAngle = 0.01;
    bool test = true;
    int _screenWidth, _screenHeight;
    double ratio;

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
    void moveMouse(float x, float y);
    void setDelta(float delta_x, float delta_y);
    void testing();
    void doubleTap(float x, float y);
    void longPress(float x, float y);
    osg::Vec3f screenToWorld(float x, float y);
};


#endif //TESTING_CALVR_APPLICATION_H
