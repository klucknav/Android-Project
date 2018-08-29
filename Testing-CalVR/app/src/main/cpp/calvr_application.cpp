#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>

// CalVR
#include <cvrMenu/MenuManager.h>
#include <cvrMenu/SubMenu.h>
#include <cvrKernel/SceneManager.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrUtil/AndroidGetenv.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/InteractionEvent.h>
#include <cvrMenu/SubMenu.h>

// ADDED
#include <cvrKernel/SceneObject.h>

#include "calvr_application.h"
#include "util.h"
#include "osg_utils.h"

using namespace osg;

calvr_application::calvr_application(AAssetManager *assetManager) : _asset_manager(assetManager) {

    LOGI("========== in CalVR app constructor =========");
    _root = new Group();

    _sphereTrans = new PositionAttitudeTransform();         // ADDED
    _cubeTrans = new PositionAttitudeTransform();           // ADDED

    _menuManager = cvr::MenuManager::instance();
    _scene = cvr::SceneManager::instance();
    _config = cvr::ConfigManager::instance();
    _viewer = new cvr::CVRViewer();
    _viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    _menuBasics = new MenuBasics();
    _spatialViz = new SpatialViz();

    _interactionManager = cvr::InteractionManager::instance();


    initialize_camera();
    LOGI("========== finished CalVR app constructor =========");
}

void calvr_application::initialize_camera() {

    osg::ref_ptr<osg::Camera> mainCam = _viewer->getCamera();
    mainCam->setClearColor(osg::Vec4f(0.81, 0.77, 0.75,1.0));
    osg::Vec3d eye = osg::Vec3d(0,-10,0);
    osg::Vec3d center = osg::Vec3d(0,0,0);
    osg::Vec3d up = osg::Vec3d(0,0,1);
    mainCam->setViewMatrixAsLookAt(eye,center,up); // usual up vector
    mainCam->setRenderOrder(osg::Camera::NESTED_RENDER);

    _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    _viewer->getCameraManipulator()->setHomePosition(eye,center,up,false);
}

void calvr_application::createDebugOSGsphere(osg::Vec3 pos) {
    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable;
    shape->setShape(new osg::Sphere(pos, 0.2f));
    shape->setColor(osg::Vec4f(0.3, 0.8, 0.8, 1.0));
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    Program * program = osg_utils::createShaderProgram("shaders/lightingOSG.vert","shaders/lightingOSG.frag",_asset_manager);
    //Program * program = osg_utils::createShaderProgram("shaders/basicOSG.vert","shaders/basicOSG.frag",_asset_manager);

    osg::StateSet * stateSet = shape->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);
    stateSet->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("shininess", 16.0f) );
    stateSet->addUniform( new osg::Uniform("lightPosition", lightDir));

    geode->addDrawable(shape.get());
    //_root->addChild(geode);       // original
    _root->addChild(_sphereTrans.get());  // ADDED
    _sphereTrans->addChild(geode);  // ADDED
}

void calvr_application::createDebugOSGcube(osg::Vec3 pos) {
    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable;
    shape->setShape(new osg::Box(pos, 0.2f));
    shape->setColor(osg::Vec4f(0.3, 0.8, 0.8, 1.0));
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    Program * program = osg_utils::createShaderProgram("shaders/lightingOSG.vert","shaders/lightingOSG.frag",_asset_manager);
    //Program * program = osg_utils::createShaderProgram("shaders/basicOSG.vert","shaders/basicOSG.frag",_asset_manager);

    osg::StateSet * stateSet = shape->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);
    stateSet->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("shininess", 16.0f) );
    stateSet->addUniform( new osg::Uniform("lightPosition", lightDir));

    geode->addDrawable(shape.get());
    //_root->addChild(geode);       // original
    _root->addChild(_cubeTrans.get());  // ADDED
    _cubeTrans->addChild(geode);  // ADDED
}


void calvr_application::setupDefaultEnvironment(const char *root_path) {
    std::string homeDir = std::string(root_path) + "/";

    setenv("CALVR_HOME", homeDir);
    setenv("CALVR_CONFIG_DIR", homeDir+"config/");
    setenv("CALVR_RESOURCE_DIR", homeDir+"resources/");
    setenv("CALVR_CONFIG_FILE", homeDir+"config/config.xml");
}

void calvr_application::onCreate(const char *calvr_path) {

    setupDefaultEnvironment(calvr_path);
    //createDebugOSGsphere(osg::Vec3(.0f, 0.0f, .0f));
    createDebugOSGcube(osg::Vec3(0,0,0));

    std::string fontfile = getenv("CALVR_RESOURCE_DIR");
    fontfile = fontfile + "arial.ttf";

    if(!_scene->init())
        LOGE("==========SCENE INITIALIZATION FAIL=========");

    _root->addChild(_scene->getSceneRoot());

    if(!_config->init())
        LOGE("==========CONFIG INITIALIZATION FAIL========");

    if(!_menuManager->init())
        LOGE("==========MENU INITIALIZATION FAIL=========");

    if(!_menuBasics->init())
        LOGE("MENU BASICS");

    if(!_spatialViz->init())
        LOGE("SPATIALVIZ INITIALIZATION FAIL");

    if(!_interactionManager->init())
        LOGE("========== INTERACTION MANAGER INIT FAIL ==========");

    _viewer->setSceneData(_root.get());
}

void calvr_application::onDrawFrame(){
    if(test) {
        setDelta(currAngle, 0);
    }
    _menuManager->update();         // to make sure the menu options gets drawn
    _interactionManager->update();  // to process the tap events
    _viewer->frame();
}

void calvr_application::onViewChanged(int rotation, int width, int height){
    _viewer->setUpViewerAsEmbeddedInWindow(0,0,width,height);
    LOGI("===== WIDTH = %d, HEIGHT = %d =====", width, height);
    _screenWidth = width;
    _screenHeight = height;
    ratio = _screenHeight/540;  // 540 is CalVR height
}

void calvr_application::onResourceLoaded(const char *path) {
    FILE* fp = fopen(path, "r");
    if(nullptr == fp){
        LOGE("FAILED TO LOAD RESOURCE FILE");
        return;
    }
}

// -------------------- mouse events -------------------- //
void calvr_application::pressMouse(bool down, float x, float y) {

    cvr::MouseInteractionEvent * interactionEvent = new cvr::MouseInteractionEvent();
    interactionEvent->setButton(0);//primary button
    osg::Matrix m;
    m.makeTranslate(screenToWorld(x,y));
    interactionEvent->setTransform(m);

    if (down) {
        interactionEvent->setInteraction(cvr::Interaction::BUTTON_DOWN);
    }
    else {
        interactionEvent->setInteraction(cvr::Interaction::BUTTON_UP);
    }
    _interactionManager->addEvent(interactionEvent);
}

void calvr_application::moveMouse(float x, float y) {
    //setDelta(x, y);       // cube disappeared when added this call
    cvr::MouseInteractionEvent * interactionEvent = new cvr::MouseInteractionEvent();
    interactionEvent->setButton(0); //primary button
    interactionEvent->setInteraction(cvr::Interaction::BUTTON_DRAG);
    osg::Matrix m;
    m.makeTranslate(screenToWorld(x,y));
    interactionEvent->setTransform(m);
    _interactionManager->addEvent(interactionEvent);
}

void calvr_application::setDelta(float delta_x, float delta_y) {
    totalRotation = totalRotation * osg::Quat(delta_y, xAxis) * osg::Quat(delta_x, zAxis);
    _cubeTrans->setAttitude(totalRotation);
}
void calvr_application::testing() {
    test = !test;
}

void calvr_application::doubleTap(float x, float y) {
    // NOTE: MouseInteractionEvent and PointerInteractionEvent both work
    cvr::MouseInteractionEvent * interactionEvent = new cvr::MouseInteractionEvent();
    interactionEvent->setButton(1); //secondary button
    osg::Matrix m;
    m.makeTranslate(screenToWorld(x, y));    // menu position
    interactionEvent->setTransform(m);
    interactionEvent->setInteraction(cvr::Interaction::BUTTON_DOUBLE_CLICK);
    _interactionManager->addEvent(interactionEvent);
}
void calvr_application::longPress(float x, float y){
    cvr::PointerInteractionEvent * interactionEvent = new cvr::PointerInteractionEvent();
    interactionEvent->setPointerType(cvr::PointerInteractionEvent::TOUCH_POINTER);
    interactionEvent->setButton(1); //secondary button
//    osg::Matrix m;
//    m.makeTranslate(screenToWorld(x,y));
//    interactionEvent->setTransform(m);
    interactionEvent->setInteraction(cvr::Interaction::BUTTON_DOWN);
    _interactionManager->addEvent(interactionEvent);
}
osg::Vec3f calvr_application::screenToWorld(float x, float y) {
//    LOGI("----- ANDROID: X = %f, Y = %f -----", x, y);

    double newX = (x-_screenWidth/2.0)/ratio;
    double newZ = (_screenHeight/2 - y)/ratio;

//    LOGI("----- CalVR: X = %f, Y = %f -----", newX, newZ);
    return osg::Vec3f(newX, 0, newZ);
}