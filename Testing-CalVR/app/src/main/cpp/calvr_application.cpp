#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>

// CalVR
#include <cvrMenu/MenuManager.h>
#include <cvrMenu/SubMenu.h>
#include <cvrKernel/SceneManager.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrUtil/AndroidHelper.h>
//#include <cvrUtil/AndroidGetenv.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/InteractionEvent.h>

// ADDED
#include <cvrKernel/SceneObject.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/ComController.h>
#include <cvrInput/TrackingManager.h>

#include "calvr_application.h"
#include "util.h"
#include "osg_utils.h"

using namespace osg;

calvr_application::calvr_application(AAssetManager *assetManager) : _asset_manager(assetManager) {

    LOGI("========== in CalVR app constructor =========");
    _root = new Group();

    // for moving the osg-object
    _sphereTrans = new PositionAttitudeTransform();         // ADDED
    _cubeTrans = new PositionAttitudeTransform();           // ADDED
    _currTrans = _cubeTrans;

    // CVR
    _viewer = new cvr::CVRViewer();
    _viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    _menuManager = cvr::MenuManager::instance();
    _scene = cvr::SceneManager::instance();
    _config = cvr::ConfigManager::instance();
    _interactionManager = cvr::InteractionManager::instance();
    _comController = cvr::ComController::instance();
    _trackingManager = cvr::TrackingManager::instance();
    _navigation = cvr::Navigation::instance();
    _pluginManager = cvr::PluginManager::instance();

    _menuBasics = new MenuBasics();
    _spatialViz = new SpatialViz();

    initialize_camera();
    LOGI("========== finished CalVR app constructor =========");
}
calvr_application::~calvr_application() {
    if(_viewer)
        _viewer = nullptr;
}
void calvr_application::initialize_camera() {

    osg::ref_ptr<osg::Camera> mainCam = _viewer->getCamera();
    mainCam->setClearColor(osg::Vec4f(0.81, 0.77, 0.75,1.0));
    osg::Vec3d eye = osg::Vec3d(0,-1000,0);
    osg::Vec3d center = osg::Vec3d(0,0,0);
    osg::Vec3d up = osg::Vec3d(0,0,1);
    mainCam->setViewMatrixAsLookAt(eye,center,up); // usual up vector
    mainCam->setRenderOrder(osg::Camera::NESTED_RENDER);

    _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    _viewer->getCameraManipulator()->setHomePosition(eye,center,up,false);
}

void calvr_application::createDebugOSGsphere(osg::Vec3 pos) {

    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable;
    shape->setShape(new osg::Sphere(pos, 0.1f));
    shape->setColor(osg::Vec4f(0.3, 0.8, 0.8, 1.0));
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    Program * program = osg_utils::createShaderProgram("shaders/lightingOSG.vert","shaders/lightingOSG.frag",_asset_manager);
    osg::StateSet * stateSet = shape->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);
    stateSet->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("shininess", 16.0f) );
    stateSet->addUniform( new osg::Uniform("lightPosition", lightDir));

    geode->addDrawable(shape.get());
//    _root->addChild(geode);                 // original
    _sphereTrans->addChild(geode);          // ADDED
    _root->addChild(_sphereTrans.get());    // ADDED
}
void calvr_application::createDebugOSGcube(osg::Vec3 pos) {
    osg::ref_ptr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable;
    shapeDrawable->setShape(new osg::Box(pos, 0.2f));
    shapeDrawable->setColor(osg::Vec4f(0.3, 0.8, 0.8, 1.0));
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    Program * program = osg_utils::createShaderProgram("shaders/lightingOSG.vert","shaders/lightingOSG.frag",_asset_manager);
    osg::StateSet * stateSet = shapeDrawable->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);
    stateSet->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("shininess", 16.0f) );
    stateSet->addUniform( new osg::Uniform("lightPosition", lightDir));

    osg::BoundingBox boundingBox = shapeDrawable.get()->getBoundingBox();
    float xMin = boundingBox.xMin();    float xMax = boundingBox.xMax();
    float yMin = boundingBox.yMin();    float yMax = boundingBox.yMax();
    float zMin = boundingBox.zMin();    float zMax = boundingBox.zMax();
    LOGI("bounding box: x = (%f, %f), y = (%f, %f), z = (%f, %f)", xMin, xMax, yMin, yMax, zMin, zMax);

    // create a scene object for the cube
    cvr::SceneObject *cubeSO = new cvr::SceneObject("root", true, false, true, true, true);
    cvr::PluginHelper::registerSceneObject(cubeSO, "CalVR_object");
    cubeSO->addChild(_root);

    LOGI("SceneObj: attaching to Scene");
//    cubeSO->attachToScene();                // fails when uncommented: fatal signal 11 (sigsegv) code 1
    LOGI("SceneObj: adding to Menu");
    cubeSO->addMoveMenuItem();
    LOGI("SceneObj: added MoveMenuItem");
    cubeSO->addNavigationMenuItem();
    LOGI("SceneObj: add NavMenuItem");

    // testing the bounding box
    cubeSO->setBoundsCalcMode(cvr::SceneObject::BoundsCalcMode::MANUAL);
    cubeSO->setBoundingBox(boundingBox);        // needs to be in MANUAL mode to work
    osg::BoundingBox SO_bb = cubeSO->getOrComputeBoundingBox();
    float xMin1 = SO_bb.xMin();    float xMax1 = SO_bb.xMax();
    float yMin1 = SO_bb.yMin();    float yMax1 = SO_bb.yMax();
    float zMin1 = SO_bb.zMin();    float zMax1 = SO_bb.zMax();
    LOGI("SO_bbox: x = (%f, %f), y = (%f, %f), z = (%f, %f)", xMin1, xMax1, yMin1, yMax1, zMin1, zMax1);

    geode->addDrawable(shapeDrawable.get());
//    _root->addChild(geode);                 // original
    _cubeTrans->addChild(geode);            // ADDED
    _root->addChild(_cubeTrans.get());      // ADDED
}


void calvr_application::setupDefaultEnvironment(const char *root_path) {
    std::string homeDir = std::string(root_path) + "/";

    setenv("CALVR_HOST_NAME", "calvrHost");
    setenv("CALVR_HOME", homeDir);
    setenv("CALVR_ICON_DIR", homeDir + "icons/");
    setenv("CALVR_CONFIG_DIR", homeDir + "config/");
    setenv("CALVR_RESOURCE_DIR", homeDir + "resources/");
    setenv("CALVR_CONFIG_FILE", homeDir + "config/config.xml");

    LOGI("--- default environment setUp Complete ---");
}
void calvr_application::onCreate(const char *calvr_path) {

    LOGI("--- onCreate ---");
    // set the environment paths
    setupDefaultEnvironment(calvr_path);


//    createDebugOSGsphere(osg::Vec3(-0.51f, 0.0f, 2.675f));
//    createDebugOSGsphere(osg::Vec3(0.0, 0.1, 0.0));
//    createDebugOSGcube(osg::Vec3(0,0,0));

    std::string fontfile = getenv("CALVR_RESOURCE_DIR");
    fontfile = fontfile + "arial.ttf";

    // Check initializations
    if(!_config->init())
        LOGE("========== CONFIG INITIALIZATION FAIL ========");
    else
        LOGI("--- CONFIG INITIALIZED ---");
    if(!_comController->init())
        LOGE("========== CONTROLLER INITIALIZATION FAIL ========");
    if(!_trackingManager->init())
        LOGE("========== TRACKING INITIALIZATION FAIL ========");
    if(!_interactionManager->init())
        LOGE("========== INTERACTION MANAGER INIT FAIL ==========");
    else
        LOGI("--- INTERACTION MANAGER INITIALIZED ---");
    if(!_navigation->init())
        LOGE("========== NAVIGATION INIT FAIL ==========");
    if(!_scene->init())
        LOGE("========== SCENE INITIALIZATION FAIL =========");
    else
        LOGI("--- SCENE INITIALIZED ---");

    _root->addChild(_scene->getSceneRoot());
    if(!_menuManager->init())
        LOGE("========== MENU INITIALIZATION FAIL =========");
    else
        LOGI("--- MENU INITIALIZED ---");

    //if(!_pluginManager->init(_asset_manager))
    //    LOGE("========== PLUG IN  FAIL =========");
    if(!_menuBasics->init())
        LOGE("MENU BASICS");
    else
        LOGI("MENU BASICS OK");

    LOGI("- about to set Scene Data - ");
    _scene->setViewerScene(_viewer);
//    _viewer->setSceneData(_root.get());   // this line has issues when SpatialViz initialized
    LOGI("- set the scene data - ");

    LOGI("about to initialize SpatialViz...");
    bool test = _spatialViz->init();
    LOGI("=============== bool test = %s ===============", test ? "true" : "false");
    if(!test)
        LOGE("SPATIALVIZ INITIALIZATION FAIL");
    else
        LOGI("SPATIAL VIZ OK");

    LOGI("=== ON CREATE COMPLETE ===");
}

void calvr_application::onDrawFrame(){

    _viewer->frameStart();
    _viewer->advance(USE_REFERENCE_TIME);

    _trackingManager->update();
    _scene->update();
    _menuManager->update();         // to make sure the menu options gets drawn
    _interactionManager->update();  // to process the tap events
    _navigation->update();
    _scene->postEventUpdate();

    //_pluginManager->preFrame();
    _spatialViz->preFrame();        // testing
    _viewer->updateTraversal();
    _viewer->renderingTraversals();
    //LOGI("here? 8");
    // OLD WAY
    //_viewer->frame();               // here is where it falters with SpatialViz
    //LOGI("here? 9");

    if(_comController->getIsSyncError())
        LOGE("Sync error");

    _pluginManager->postFrame();
    LOGI("FINISHED");
}

void calvr_application::onViewChanged(int rotation, int width, int height){

    _viewer->setUpViewerAsEmbeddedInWindow(0,0,width,height);
    LOGI("===== WIDTH = %d, HEIGHT = %d =====", width, height);
    _screenWidth = width;
    _screenHeight = height;
//    ratio = height/540/3;          // 540 is CalVR height
    ratio = height/2134;         // 2134 is updated CalVR height (with camera eye at (0,-1000,0))
    objRatio = _screenHeight/5.35;      // for osgObj location
    LOGI("===== ratio = %f =====", ratio);
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
    if (down) {
        interactionEvent->setInteraction(cvr::Interaction::BUTTON_DOWN);
        LOGI("=== BUTTON DOWN ===");
    }
    else {
        interactionEvent->setInteraction(cvr::Interaction::BUTTON_UP);
        LOGI("=== BUTTON UP ===");
    }

    interactionEvent->setButton(0);//primary button
    interactionEvent->setHand(0);
    interactionEvent->setX(x);
    interactionEvent->setY(y);
    osg::Matrix m;
    Vec3 translation = screenToWorld(x, y);
    //translation /= 1000.0;               // can't do division here bc touch event won't be in the correct place
    m.makeTranslate(translation);
    interactionEvent->setTransform(m);
    _trackingManager->setTouchEventMatrix(m);
    _interactionManager->addEvent(interactionEvent);
}

void calvr_application::moveMouse(float delta_x, float delta_y, float x, float y) {

//    LOGI("calvr_app - moveMouse: x = %f, y = %f", x, y);
//    LOGI("calvr_app - moveMouse: Dx = %f, Dy = %f", delta_x, delta_y);

    /*if(moveOBJ) {
        if (currMode == MOVE_WORLD) {
            LOGI("calvr_app - MOVE_WORLD MODE");
            totalRotation = totalRotation * osg::Quat(delta_y, xAxis) * osg::Quat(delta_x, zAxis);
            _currTrans->setAttitude(totalRotation);
        }
        if (currMode == SCALE) {
            LOGI("calvr_app - SCALE MODE");
            osg::Vec3 prevScale = _currTrans->getScale();
            float deltaReduced = delta_y;
            osg::Vec3 currScale = osg::Vec3(deltaReduced, deltaReduced, deltaReduced);

            // make sure the next scale is POS/doesn't get too small/doesn't get too large
            osg::Vec3 nextScale = prevScale - currScale;
            if (nextScale[0] < 0.01)
                return;
            if (nextScale[0] > 10)
                return;
            // set the new scale
            _currTrans->setScale(prevScale - currScale);
        }

        if (currMode == DRIVE) {
            LOGI("calvr_app - DRIVE MODE");
            osg::Vec3 prevPos = _currTrans->getPosition();
            LOGI("calvr_app - moveMouse: prevPos: x = %f, y = %f, z = %f", prevPos[0], prevPos[1], prevPos[3]);
            osg::Vec3 move = osg::Vec3(delta_x / 2.5f, 0, 0);
            LOGI("calvr_app - moveMouse: move: x = %f, y = %f, z = %f", move[0], move[1], move[3]);
            _currTrans->setPosition(prevPos + move);
        }

        if (currMode == FLY) {
            LOGI("calvr_app - FLY MODE");
            osg::Vec3 prevPos = _currTrans->getPosition();
            osg::Vec3 move = osg::Vec3(delta_x / 2.5f, 0, -delta_y / 2.5f);
            _currTrans->setPosition(prevPos + move);
        }
    }*/
    cvr::MouseInteractionEvent * interactionEvent = new cvr::MouseInteractionEvent();
    interactionEvent->setButton(0); //primary button
    interactionEvent->setHand(0);
    interactionEvent->setX(x);    // no diff with coord from screenToWorld and x
    interactionEvent->setY(y);
    interactionEvent->setInteraction(cvr::Interaction::BUTTON_DRAG);
    LOGI("=== BUTTON DRAG === %f, %f", x, y);
    osg::Matrix m;
    Vec3 translation = screenToWorld(x, y);     // testing delta's instead of x and y
    m.makeTranslate(translation);               // TODO try screenToWorldObj - no visible change
    interactionEvent->setTransform(m);
    _trackingManager->setTouchMovePosition(x, y);       // TESTING
    _interactionManager->addEvent(interactionEvent);
}


void calvr_application::setDelta(float delta_x, float delta_y) {
    totalRotation = totalRotation * osg::Quat(delta_y, xAxis) * osg::Quat(delta_x, zAxis);
    _cubeTrans->setAttitude(totalRotation);
}

// open and close the menu
void calvr_application::doubleTap(float x, float y) {

    LOGI("calvr_app - doubleTap: x = %f, y = %f", x, y);
    cvr::MouseInteractionEvent * interactionEvent = new cvr::MouseInteractionEvent();
    interactionEvent->setButton(1); //secondary button
    interactionEvent->setInteraction(cvr::Interaction::BUTTON_DOUBLE_CLICK);
    interactionEvent->setX(x);
    interactionEvent->setY(y);
    interactionEvent->setHand(0);
    osg::Matrix m;
    osg::Vec3 menuPos = screenToWorld(x, y);
//    menuPos = Vec3(-596,0,-1060);
    m.makeTranslate(menuPos);    // menu position
    interactionEvent->setTransform(m);
    _trackingManager->setTouchEventMatrix(m);

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

// move modes
void calvr_application::switchMoveMode() {
    LOGI("Switched Mode to: %s", moveOBJ ? "off" : "on");
    moveOBJ = !moveOBJ;
}
void calvr_application::setMode(int newMode) {
    currMode = newMode;
}
void calvr_application::reset() {
    _currTrans->setScale(defaultScale);
    _currTrans->setPosition(defaultPos);
    _currTrans->setAttitude(defaultRot);
}

void calvr_application::leftClick(float x, float y){
    LOGI("----- LEFT CLICK -----");
    cvr::MouseInteractionEvent * interactionEvent = new cvr::MouseInteractionEvent();
    interactionEvent->setButton(1); //secondary button
    interactionEvent->setInteraction(cvr::Interaction::BUTTON_DOWN);
    interactionEvent->setX(x);
    interactionEvent->setY(y);
    interactionEvent->setHand(0);
    osg::Matrix m;
    osg::Vec3 menuPos = screenToWorld(x, y);
//    menuPos = Vec3(-596,0,-1060);
    m.makeTranslate(menuPos);    // menu position
    interactionEvent->setTransform(m);
    _trackingManager->setTouchEventMatrix(m);

    _interactionManager->addEvent(interactionEvent);
}


// helper functions
osg::Vec3f calvr_application::screenToWorld(float x, float y) {
//    LOGI("----- ANDROID: X = %f, Y = %f -----", x, y);

    double newX = (x -_screenWidth/2.0)/0.9;
    double newZ = (_screenHeight/2 - y)/0.9;

//    LOGI("----- CalVR: X = %f, Y = %f -----", newX, newZ);
    return osg::Vec3f(newX, 0, newZ);
}
osg::Vec3 calvr_application::screenToWorldObj(float x, float y){
//    LOGI("----- ANDROID: X = %f, Y = %f -----", x, y);

    double newX = (x -_screenWidth/2.0)/objRatio;
    double newZ = (_screenHeight/2 - y)/objRatio;

//    LOGI("----- CalVR: X = %f, Y = %f -----", newX, newZ);
    return osg::Vec3f(newX, 0, newZ);
}