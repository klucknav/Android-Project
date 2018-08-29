#ifndef TESTING_AR_APPLICATION_H_
#define TESTING_AR_APPLICATION_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "arcore_c_api.h"
#include "background_renderer.h"
#include "glm.h"
//#include "obj_renderer.h"
//#include "plane_renderer.h"
//#include "point_cloud_renderer.h"
#include "pointCloudDrawable.h"
#include "util.h"

typedef  struct{
    float intensity = 0.8f;
    float color_correction[4] = {1.f, 1.f, 1.f, 1.f};
}LightSrc;

typedef struct {
    std::unordered_map<ArPlane*, glm::vec3> plane_color_map;    // Stores the randomly-selected color each plane is drawn with
    bool this_is_first_plane = true;
}PlaneParams;

// ArApplication handles all application logics.
class ArApplication {

private:
    ArSession* _ar_session = nullptr;
    ArFrame* _ar_frame = nullptr;
    ArTrackingState camera_tracking_state;

    bool install_requested_ = false;
    int _width = 1;
    int _height = 1;
    int _displayRotation = 0;
    bool first_plane_has_been_found_ = false;
    int32_t plane_count_ = 0;

    AAssetManager* const asset_manager_;

    // The anchors at which we are drawing android models using given colors.
    struct ColoredAnchor {
        ArAnchor* anchor;
        float color[4];
    };
    std::vector<ColoredAnchor> anchors_;

    // different objects to render
    BackgroundRenderer background_renderer_;
    //PointCloudRenderer point_cloud_renderer_;
    //PlaneRenderer plane_renderer_;
    //ObjRenderer andy_renderer_;

    // added for osg integration
    LightSrc _light;
    PlaneParams _planes;


    // helper function
    void SetColor(float r, float g, float b, float a, float* color4f);


public:
    glm::mat4 view_mat;
    glm::mat4 projection_mat;
    float transformed_camera_uvs[8];

    ArApplication(AAssetManager* asset_manager);
    ~ArApplication();

    void OnPause();
    void OnResume(void* env, void* context, void* activity);
    void OnSurfaceCreated();
    void OnDisplayGeometryChanged(int rotation, int width, int height);
    void OnDrawFrame();                     // original version
    void onDrawFrame(GLuint textureId);     // scaled down version

    // Touch events
    void OnTouched(float x, float y);
    void touchedPose(float x, float y, float* out_pose_raw);

    bool HasDetectedPlanes() const { return plane_count_ > 0; }
    ArSession * getSession(){ return  _ar_session;}

    // below are added for osg integration
    float* updateBackgroundRender();    // called when osg draws the background
    bool renderPointClouds(pointCloudDrawable *drawable);
    bool isTracking(){
        return (camera_tracking_state == AR_TRACKING_STATE_TRACKING);
    }
    PlaneParams detectPlanes();
    void estimateLight();               // called when osg draws planes
};

#endif  // TESTING_AR_APPLICATION_H_
