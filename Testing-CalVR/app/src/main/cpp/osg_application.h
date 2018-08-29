#ifndef TESTING_OSGAPPLICATION_H
#define TESTING_OSGAPPLICATION_H

// OSG
#include <osgViewer/Viewer>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osgGA/TrackballManipulator>

#include <unordered_map>
#include "util.h"
#include "osg_utils.h"

#include "ar_application.h"
#include "osg_background.h"

enum{
    MOVE_WORLD, // 0
    SCALE,      // 1
    DRIVE,      // 2
    FLY         // 3
};

namespace osgApp{
    class osgApplication{
    private:
        AAssetManager *const _asset_manager;

        int _plane_num = 0;
        float _color_correction[4] = {1.f, 1.f, 1.f, 1.f};
        std::unordered_map<ArPlane*,  glm::vec3> _plane_color_map;

        osgViewer::Viewer * _viewer;
        osg::ref_ptr<osg::Group>  _root;
        osg::ref_ptr<osg::Group> _sceneGroup;
        osg::ref_ptr<osg::Node> scene;

        ArApplication* _arCoreApp;
        osg::Program* program;

        // background objects
        osg::ref_ptr<backgroundDrawable> _background;

        // ADDED
        osg::ref_ptr<osg::PositionAttitudeTransform> _sphereTrans;
        osg::ref_ptr<osg::PositionAttitudeTransform> _cubeTrans;
        osg::Vec3 xAxis = osg::Vec3(1.0, 0.0, 0.0);
        osg::Vec3 yAxis = osg::Vec3(0.0, 1.0, 0.0);
        osg::Vec3 zAxis = osg::Vec3(0.0, 0.0, 1.0);
        osg::Quat totalRotation = osg::Quat(0, xAxis, 0, yAxis, 0, zAxis);
        int currMode = DRIVE;
        osg::Vec3 defaultScale = osg::Vec3(1,1,1);
        osg::Vec3 defaultPos = osg::Vec3(0,0,0);
        osg::Quat defaultRot = osg::Quat(0, xAxis, 0, yAxis, 0, zAxis);

        void create_osg_sphere(osg::Vec3 pos);
        void create_osg_cube(osg::Vec3 pos);
        void _initialize_camera();

    public:
        osgApplication(AAssetManager * manager);
        ~osgApplication();

        void onCreate();
        void onPause();
        void onResume(void *env, void* context, void *activity);

        void onDrawFrame();
        void onViewChanged(int rotation, int width, int height);
        void onTouched(float x, float y);
        void onLongPress(float x, float y);
        void moveMouse(float x, float y);
        void pressMouse(bool down, float x, float y);
        void setDelta(float delta_x, float delta_y, float x, float y);

        bool hasDetectedPlane(){ return _plane_num >0; }
        void loadScene(const char* filename);
        void setMode(int newMode);
        void reset();
    };
}

#endif //TESTING_OSGAPPLICATION_H
