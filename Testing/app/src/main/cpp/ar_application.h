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
#include "util.h"

typedef  struct{
    float intensity = 0.8f;
    float color_correction[4] = {1.f, 1.f, 1.f, 1.f};
}LightSrc;

// ArApplication handles all application logics.
class ArApplication {
public:

    ArApplication(AAssetManager* asset_manager);
    ~ArApplication();

    // OnPause is called on the UI thread from the Activity's onPause method.
    void OnPause();

    // OnResume is called on the UI thread from the Activity's onResume method.
    void OnResume(void* env, void* context, void* activity);

    // OnSurfaceCreated is called on the OpenGL thread when GLSurfaceView is created.
    void OnSurfaceCreated();

    // OnDisplayGeometryChanged is called on the OpenGL thread when the
    // render surface size or display rotation changes.
    //
    // @param display_rotation: current display rotation.
    // @param width: width of the changed surface view.
    // @param height: height of the changed surface view.
    void OnDisplayGeometryChanged(int display_rotation, int width, int height);

    // OnDrawFrame is called on the OpenGL thread to render the next frame.
    void OnDrawFrame();
    void onDrawFrame(GLuint textureId);

    // OnTouched is called on the OpenGL thread after the user touches the screen.
    // @param x: x position on the screen (pixels).
    // @param y: y position on the screen (pixels).
    void OnTouched(float x, float y);
    void touchedPose(float x, float y, float* out_pose_raw);

    // Returns true if any planes have been detected. Used for hiding the "searching for planes" snackbar.
    bool HasDetectedPlanes() const { return plane_count_ > 0; }


    // below are added for osg integration
    float* updateBackgroundRender();    // called when osg draws the background
    void estimateLight();               // called when osg draws planes
    float transformed_camera_uvs[8];

private:
    ArSession* _ar_session = nullptr;
    ArFrame* _ar_frame = nullptr;

    bool install_requested_ = false;
    int width_ = 1;
    int height_ = 1;
    int display_rotation_ = 0;

    AAssetManager* const asset_manager_;

    // The anchors at which we are drawing android models using given colors.
    struct ColoredAnchor {
        ArAnchor* anchor;
        float color[4];
    };

    std::vector<ColoredAnchor> anchors_;

    // Stores the randomly-selected color each plane is drawn with
    std::unordered_map<ArPlane*, glm::vec3> plane_color_map_;

    // The first plane is always rendered in white, if this is true then a plane
    // at some point has been found.
    bool first_plane_has_been_found_ = false;

    BackgroundRenderer background_renderer_;
    //PointCloudRenderer point_cloud_renderer_;
    //PlaneRenderer plane_renderer_;
    //ObjRenderer andy_renderer_;

    int32_t plane_count_ = 0;

    void SetColor(float r, float g, float b, float a, float* color4f);


    // below are added for osg integration
    LightSrc _light;
};

#endif  // TESTING_AR_APPLICATION_H_
