#include "osg_application.h"

using namespace osgApp;

osgApplication::osgApplication(AAssetManager * manager) : _asset_manager(manager){

    _viewer = new osgViewer::Viewer();

    // use single thread: critical for mobile and web
    _viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    _root = new osg::Group();
    _sceneGroup = new osg::Group();

    // ADDED
    _sphereTrans = new osg::PositionAttitudeTransform();
    _cubeTrans = new osg::PositionAttitudeTransform();

    _initialize_camera();
    _arCoreApp = new ArApplication(manager);
}

osgApplication::~osgApplication() {
    delete(_viewer);
}

void osgApplication::_initialize_camera() {
    osg::Camera *mainCamera = _viewer->getCamera();
    mainCamera->setClearColor(osg::Vec4(0.81, 0.77, 0.75, 1.0));

    // set position and orientation of the viewer
    osg::Vec3d eye = osg::Vec3d(0,-5,0);
    osg::Vec3d center = osg::Vec3d(0,0,.0);
    osg::Vec3d up = osg::Vec3d(0,0,1);
    mainCamera->setViewMatrixAsLookAt(eye,center,up);
    mainCamera->setRenderOrder(osg::Camera::NESTED_RENDER);

    _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    _viewer->getCameraManipulator()->setHomePosition(eye,center,up,false);
}

void osgApplication::create_osg_sphere(osg::Vec3 pos) {

    osg::Sphere *unitSphere = new osg::Sphere(pos, 0.1f);
    osg::ShapeDrawable *sphereDrawable = new osg::ShapeDrawable(unitSphere);
    sphereDrawable->setColor(osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
    osg::Geode *sphereGeode = new osg::Geode();

    // Need otherwise doesn't draw
    Program * program = osg_utils::createShaderProgram("shaders/basicOSG.vert","shaders/basicOSG.frag",_asset_manager);
    osg::StateSet * stateSet = sphereDrawable->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);

    sphereGeode->addDrawable(sphereDrawable);
    //_sceneGroup->addChild(sphereGeode);   // original
    _sceneGroup->addChild(_sphereTrans);    // ADDED
    _sphereTrans->addChild(sphereGeode);    // ADDED
    //_root->addChild(_sphereTrans);          // ADDED - cannot see the sphere
    //_root->addChild(sphereGeode);           // testing - cannot see the sphere
}
void osgApplication::create_osg_cube(osg::Vec3 pos) {

    osg::Box *unitBox = new osg::Box(pos, 0.2f);
    osg::ShapeDrawable *boxDrawable = new osg::ShapeDrawable(unitBox);
    osg::Geode *boxGeode = new osg::Geode();

    Program *program1 = osg_utils::createShaderProgram("shaders/basicOSG.vert", "shaders/basicOSG.frag", _asset_manager);

    osg::StateSet *stateSet = boxDrawable->getOrCreateStateSet();
    stateSet->setAttribute(program1);
    boxGeode->addDrawable(boxDrawable);
    //_sceneGroup->addChild(boxGeode);

    _cubeTrans->addChild(boxGeode);     // ADDED
    _root->addChild(_cubeTrans);        // ADDED

}

void osgApplication::onCreate() {

    // create a sphere and add it to the root
    create_osg_sphere(osg::Vec3(0.0f,0.0f,0.0f));

    // initialize the background
    _background = new backgroundDrawable();
    _background->init(_asset_manager);
    _root->addChild(_background->createDrawableNode());       // UNCOMMENT to add the AR-background

    // initialize the point cloud
    //_pointcloudDrawable = new pointDrawable();
    //_sceneGroup->addChild(_pointcloudDrawable->createDrawableNode(_asset_manager, &glStateStack));

    // initialize the object
    //_sceneGroup->addChild(_object_renderer->createNode(_asset_manager, "models/andy.obj", "textures/andy.png"));

    // make sure the camera image is always in the back
    program = osg_utils::createShaderProgram("shaders/basicOSG.vert", "shaders/basicOSG.frag");
    _sceneGroup->getOrCreateStateSet()->setAttribute(program);
    _sceneGroup->getOrCreateStateSet()->setRenderBinDetails(2,"RenderBin");
    _sceneGroup->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
    _root->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    _root->addChild(_sceneGroup.get());
    _viewer->setSceneData(_root.get());               // original
    //_viewer->setSceneData(_sceneGroup.get());       // testing - can see the sphere again but not the background
}

void osgApplication::onPause() {
    _arCoreApp->OnPause();
}
void osgApplication::onResume(void *env, void *context, void *activity) {
    _arCoreApp->OnResume(env, context, activity);
}

void osgApplication::onDrawFrame() {
    _arCoreApp->onDrawFrame(_background->GetTextureId());

    // draw the background
    float* transUV = _arCoreApp->updateBackgroundRender();
    if (transUV != nullptr){
        _background->updateUVs(transUV);
    }

    _viewer->frame();
}

void osgApplication::onViewChanged(int rotation, int width, int height) {
    _viewer->setUpViewerAsEmbeddedInWindow(0, 0, width, height);
    _arCoreApp->OnDisplayGeometryChanged(rotation, width, height);
}

void osgApplication::onTouched(float x, float y) {
    create_osg_sphere(osg::Vec3(x, 0.0, y));
    _arCoreApp->OnTouched(x, y);
}
void osgApplication::onLongPress(float x, float y){
    create_osg_cube(osg::Vec3(x, 0.0, y));
    //_arCoreApp->OnTouched(x, y);  // when not commented out fine but no cube
                                    // when commented out fails... sometimes

    //float* ar_raw_pose;
    //LOGI("---------- calling touchedPose ----------");
    //_arCoreApp->touchedPose(x, y, ar_raw_pose);
    //LOGI("---------- got the pose ----------");
    //float arX = ar_raw_pose[4];
    //float arY = ar_raw_pose[5];
    //float arZ = ar_raw_pose[6];

//    const char* message = (std::to_string(arX)).c_str();
//    LOGI(message); // causes errors
    //std::string messageX = " x = " + std::to_string(arX);
    //std::string mexxageY = " y = " + std::to_string(arY);
    //std::string messageZ = " z = " +std::to_string(arZ);
    //LOGI(messageX.c_str());
    //create_osg_cube(osg::Vec3(arX/1000.0, arY/1000.0, arZ/1000.0));
}


void osgApplication::moveMouse(float x, float y){
    _viewer->getEventQueue()->mouseMotion(x, y);
}
void osgApplication::pressMouse(bool down, float x, float y) {
    if (down)
        _viewer->getEventQueue()->mouseButtonPress(x, y, 2);
    else
        _viewer->getEventQueue()->mouseButtonRelease(x, y, 2);
}
void osgApplication::setDelta(float delta_x, float delta_y, float x, float y){
    if (currMode == MOVE_WORLD) {
        totalRotation = totalRotation * osg::Quat(delta_y, xAxis) * osg::Quat(delta_x, zAxis);
        _sphereTrans->setAttitude(totalRotation);
    }
    if (currMode == SCALE){
        osg::Vec3 prevScale = _sphereTrans->getScale();
        float deltaReduced = delta_y;
        osg::Vec3 currScale = osg::Vec3(deltaReduced, deltaReduced, deltaReduced);

        // make sure the next scale is POS/doesn't get too small/doesn't get too large
        osg::Vec3 nextScale = prevScale - currScale;
        if (nextScale[0] < 0.01)
            return;
        if (nextScale[0] > 10)
            return;

        // set the new scale
        _sphereTrans->setScale(prevScale - currScale);
    }
    if (currMode == DRIVE){
        osg::Vec3 prevPos = _sphereTrans->getPosition();
        osg::Vec3 move = osg::Vec3(delta_x/5.0f, 0, 0);
        _sphereTrans->setPosition(prevPos + move);
    }
    if (currMode == FLY){
        osg::Vec3 prevPos = _sphereTrans->getPosition();
        osg::Vec3 move = osg::Vec3(delta_x/5.0f, 0, -delta_y/5.0f);
        _sphereTrans->setPosition(prevPos + move);
    }
}
void osgApplication::setMode(int newMode) {
    currMode = newMode;
}
void osgApplication::reset() {
    _sphereTrans->setScale(defaultScale);
    _sphereTrans->setPosition(defaultPos);
    _sphereTrans->setAttitude(defaultRot);
}


void osgApplication::loadScene(const char *filename) {

    scene = osgDB::readNodeFile(filename);
    if (!scene){
        osg::notify(osg::FATAL) << "Could not load scene";
        return;
    }
    _root->addChild(scene.get());
}

