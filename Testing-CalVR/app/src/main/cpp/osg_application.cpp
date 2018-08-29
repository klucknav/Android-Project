#include "osg_application.h"

using namespace osgApp;

osgApplication::osgApplication(AAssetManager * manager) : _asset_manager(manager){

    _viewer = new osgViewer::Viewer();  // use single thread: critical for mobile and web
    _viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    _root = new osg::Group();
    _sceneGroup = new osg::Group();

    _initialize_camera();

    // For OSG object manupulation
    _sphereTrans = new osg::PositionAttitudeTransform();
    _cubeTrans = new osg::PositionAttitudeTransform();

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
    mainCamera->setCullingMode(osg::CullSettings::NO_CULLING);      // background disappears otherwise->bbox issue

    // comment out to get osg - object in space
//    _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
//    _viewer->getCameraManipulator()->setHomePosition(eye,center,up,false);
}
void osgApplication::create_osg_sphere(osg::Vec3 pos) {

    osg::ref_ptr<osg::Sphere> unitSphere = new osg::Sphere(pos, 0.1f);
    osg::ref_ptr<osg::ShapeDrawable> sphereDrawable = new osg::ShapeDrawable(unitSphere);
    sphereDrawable->setColor(osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
    osg::ref_ptr<osg::Geode> sphereGeode = new osg::Geode();

    Program * program = osg_utils::createShaderProgram("shaders/lightingOSG.vert","shaders/lightingOSG.frag",_asset_manager);
    osg::StateSet * stateSet = sphereDrawable->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);
    stateSet->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("shininess", 16.0f) );
    stateSet->addUniform( new osg::Uniform("lightPosition", lightDir));

    sphereGeode->addDrawable(sphereDrawable);
    //_sceneGroup->addChild(sphereGeode);   // original
    _sceneGroup->addChild(_sphereTrans);    // ADDED
    _sphereTrans->addChild(sphereGeode);    // ADDED
}
void osgApplication::create_osg_cube(osg::Vec3 pos) {

    osg::ref_ptr<osg::Box> unitBox = new osg::Box(pos, 0.2f);
    osg::ref_ptr<osg::ShapeDrawable> boxDrawable = new osg::ShapeDrawable(unitBox);
    osg::ref_ptr<osg::Geode> boxGeode = new osg::Geode();

    Program * program = osg_utils::createShaderProgram("shaders/lightingOSG.vert","shaders/lightingOSG.frag",_asset_manager);
    osg::StateSet * stateSet = boxDrawable->getOrCreateStateSet();
    stateSet->setAttributeAndModes(program);
    stateSet->addUniform( new osg::Uniform("lightDiffuse", osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("lightSpecular", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)) );
    stateSet->addUniform( new osg::Uniform("shininess", 16.0f) );
    stateSet->addUniform( new osg::Uniform("lightPosition", lightDir));

    boxGeode->addDrawable(boxDrawable);
    //_sceneGroup->addChild(boxGeode);      // original
    _sceneGroup->addChild(_cubeTrans);      // ADDED
    _cubeTrans->addChild(boxGeode);         // ADDED
}

void osgApplication::onCreate() {

    // create an osg object and add it to the root
    create_osg_cube(cubePos);
    _currTrans = _cubeTrans;

    // initialize the background
    if(drawBackground) {
        _background = new backgroundDrawable();
        _background->init(_asset_manager);
        _root->addChild(_background->createDrawableNode());

        // make sure the camera image is always in the back
        _sceneGroup->getOrCreateStateSet()->setRenderBinDetails(2, "RenderBin");
        _sceneGroup->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
        _root->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    }

    // initialize the point cloud
    if(drawPointCloud){
        _pointCloud = new pointCloudDrawable();
        _pointCloud->init(_asset_manager);
        _sceneGroup->addChild(_pointCloud->createDrawableNode());
    }

    // initialize Andy
    if(drawAndy) {
        LOGI("----- INITIALIZING ANDY -----");
        _andy = new osg_objRenderer;
        LOGI("----- ANDY INITIALIZED -----");
        _sceneGroup->addChild(_andy->createNode(_asset_manager,
                                                "models/andy.obj", "models/andy_green.png"));
        LOGI("----- ADDED ANDY TO THE SCENE GROUP -----");
    }

    _root->addChild(_sceneGroup.get());
    _viewer->setSceneData(_root.get());
}
void osgApplication::onPause() {
    _arCoreApp->OnPause();
}
void osgApplication::onResume(void *env, void *context, void *activity) {
    _arCoreApp->OnResume(env, context, activity);
}
void osgApplication::onDrawFrame() {

    // update the AR info
    _arCoreApp->onDrawFrame(_background->GetTextureId());

    // update the camera position to match the camera from AR-Core
    if(inWorldSpace) {
        glm::mat4 rotatemat = glm::rotate(glm::mat4(), glm::radians(-90.0f),
                                          glm::vec3(1.0, 0, 0)) * _arCoreApp->view_mat;
        osg::Matrixd *mat = new osg::Matrixd(glm::value_ptr(rotatemat));
        _viewer->getCamera()->setViewMatrix(*mat);
    }

    // update and draw the point cloud
    if(drawPointCloud) {
        _arCoreApp->renderPointClouds(_pointCloud);
    }

    // update and draw Andy
    if(drawAndy) {
        _andy->Draw(_arCoreApp->projection_mat, _arCoreApp->view_mat,
                    glm::translate(glm::mat4(), andyPos), _color_correction, 1);
    }

    // update and draw the background
    float* transUV = _arCoreApp->updateBackgroundRender();
    if (transUV != nullptr){
        _background->updateUVs(transUV);
    }

    if(drawPlanes){
        if(!_arCoreApp->isTracking()){
            return;
        }
        _arCoreApp->estimateLight();

        PlaneParams planes = _arCoreApp->detectPlanes();
        int numPlanes = (int) planes.plane_color_map.size();
        if(numPlanes > 0) {
            LOGI("----- PLANES: planes detected(%d) -----", numPlanes);
        }
        if(_plane_num < planes.plane_color_map.size()){
            for(int i = _plane_num; i < planes.plane_color_map.size(); i++){
                planeDrawable *currPlane = new planeDrawable();
                _sceneGroup->addChild((currPlane->createDrawableNode()));
                _planeDrawables.push_back(currPlane);
            }
            _plane_num = (int) planes.plane_color_map.size();
        }

        auto plantIt = planes.plane_color_map.begin();
        for(int i = 0; i < _plane_num; i++, plantIt++){
            _planeDrawables[i]->updateOnFrame(_arCoreApp->getSession(), plantIt->first,
                                              _arCoreApp->projection_mat, _arCoreApp->view_mat, plantIt->second);
        }
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
        _currTrans->setAttitude(totalRotation);
    }
    if (currMode == SCALE){
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
    if (currMode == DRIVE){
        osg::Vec3 prevPos = _currTrans->getPosition();
        osg::Vec3 move = osg::Vec3(delta_x/5.0f, 0, 0);
        _currTrans->setPosition(prevPos + move);
    }
    if (currMode == FLY){
        osg::Vec3 prevPos = _currTrans->getPosition();
        osg::Vec3 move = osg::Vec3(delta_x/5.0f, 0, -delta_y/5.0f);
        _currTrans->setPosition(prevPos + move);
    }
}
void osgApplication::setMode(int newMode) {
    currMode = newMode;
}
void osgApplication::reset() {
    _currTrans->setScale(defaultScale);
    _currTrans->setPosition(defaultPos);
    _currTrans->setAttitude(defaultRot);
}
void osgApplication::switchView(){
    LOGI("OSG_APP: SwtichView set to %s", inWorldSpace ? "false" : "true");
    inWorldSpace = !inWorldSpace;
}

void osgApplication::loadScene(const char *filename) {

    scene = osgDB::readNodeFile(filename);
    if (!scene){
        osg::notify(osg::FATAL) << "Could not load scene";
        return;
    }
    _root->addChild(scene.get());
}

