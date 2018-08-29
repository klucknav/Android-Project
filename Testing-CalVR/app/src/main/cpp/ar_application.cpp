/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ar_application.h"

#include <android/asset_manager.h>
#include <array>


#include "util.h"


  namespace {
  constexpr size_t kMaxNumberOfAndroidsToRender = 20;
  constexpr int32_t kPlaneColorRgbaSize = 16;

  const glm::vec3 kWhite = {255, 255, 255};

  constexpr std::array<uint32_t, kPlaneColorRgbaSize> kPlaneColorRgba = {
      {0xFFFFFFFF, 0xF44336FF, 0xE91E63FF, 0x9C27B0FF, 0x673AB7FF, 0x3F51B5FF,
       0x2196F3FF, 0x03A9F4FF, 0x00BCD4FF, 0x009688FF, 0x4CAF50FF, 0x8BC34AFF,
       0xCDDC39FF, 0xFFEB3BFF, 0xFFC107FF, 0xFF9800FF}};

  inline glm::vec3 GetRandomPlaneColor() {
    const int32_t colorRgba = kPlaneColorRgba[std::rand() % kPlaneColorRgbaSize];
    return glm::vec3(((colorRgba >> 24) & 0xff) / 255.0f,
                     ((colorRgba >> 16) & 0xff) / 255.0f,
                     ((colorRgba >> 8) & 0xff) / 255.0f);
    }
  }  // namespace

ArApplication::ArApplication(AAssetManager* asset_manager) : asset_manager_(asset_manager) {}

ArApplication::~ArApplication() {
    if (_ar_session != nullptr) {
        ArSession_destroy(_ar_session);
        ArFrame_destroy(_ar_frame);
    }
}

void ArApplication::OnPause() {
    LOGI("OnPause()");
    if (_ar_session != nullptr) {
        ArSession_pause(_ar_session);
    }
}

void ArApplication::OnResume(void* env, void* context, void* activity) {
    LOGI("OnResume()");

    if (_ar_session == nullptr) {
        ArInstallStatus install_status;
        // If install was not yet requested, that means that we are resuming the
        // activity first time because of explicit user interaction (such as
        // launching the application)
        bool user_requested_install = !install_requested_;

        CHECK(ArCoreApk_requestInstall(env, activity, user_requested_install, &install_status) == AR_SUCCESS);

        switch (install_status) {
            case AR_INSTALL_STATUS_INSTALLED:
                break;
            case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                install_requested_ = true;
                return;
        }

        CHECK(ArSession_create(env, context, &_ar_session) == AR_SUCCESS);
        CHECK(_ar_session);

        ArFrame_create(_ar_session, &_ar_frame);
        CHECK(_ar_frame);

        ArSession_setDisplayGeometry(_ar_session, display_rotation_, width_, height_);
    }
    const ArStatus status = ArSession_resume(_ar_session);
    CHECK(status == AR_SUCCESS);
}

void ArApplication::OnSurfaceCreated() {
    LOGI("OnSurfaceCreated()");

    background_renderer_.InitializeGlContent(asset_manager_);
    //point_cloud_renderer_.InitializeGlContent(asset_manager_);
    //andy_renderer_.InitializeGlContent(asset_manager_, "models/andy.obj", "models/andy.png");
    //plane_renderer_.InitializeGlContent(asset_manager_);
}

void ArApplication::OnDisplayGeometryChanged(int display_rotation, int width, int height) {

    LOGI("OnSurfaceChanged(%d, %d)", width, height);

    //glViewport(0, 0, width, height);  // removed - taken care of by osg_application (setUpViewerAsEmbeddedInWindow)
    display_rotation_ = display_rotation;
    width_ = width;
    height_ = height;
    if (_ar_session != nullptr) {
        ArSession_setDisplayGeometry(_ar_session, display_rotation, width, height);
    }
}

void ArApplication::OnDrawFrame() {

    // Render the scene.
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (_ar_session == nullptr) return;

    ArSession_setCameraTextureName(_ar_session, background_renderer_.GetTextureId());

    // Update session to get current frame and render camera background.
    if (ArSession_update(_ar_session, _ar_frame) != AR_SUCCESS) {
        LOGE("ArApplication::OnDrawFrame ArSession_update error");
    }

    ArCamera* ar_camera;
    ArFrame_acquireCamera(_ar_session, _ar_frame, &ar_camera);

    view_mat;
    projection_mat;
    ArCamera_getViewMatrix(_ar_session, ar_camera, glm::value_ptr(view_mat));
    ArCamera_getProjectionMatrix(_ar_session, ar_camera, /*near=*/0.1f, /*far=*/100.f, glm::value_ptr(projection_mat));

    ArTrackingState camera_tracking_state;
    ArCamera_getTrackingState(_ar_session, ar_camera, &camera_tracking_state);
    ArCamera_release(ar_camera);

    background_renderer_.Draw(_ar_session, _ar_frame);

    // If the camera isn't tracking don't bother rendering other objects.
    if (camera_tracking_state != AR_TRACKING_STATE_TRACKING) {
        return;
    }

    // Get light estimation value.
    ArLightEstimate* ar_light_estimate;
    ArLightEstimateState ar_light_estimate_state;
    ArLightEstimate_create(_ar_session, &ar_light_estimate);

    ArFrame_getLightEstimate(_ar_session, _ar_frame, ar_light_estimate);
    ArLightEstimate_getState(_ar_session, ar_light_estimate, &ar_light_estimate_state);

    // Set light intensity to default. Intensity value ranges from 0.0f to 1.0f.
    // The first three components are color scaling factors.
    // The last one is the average pixel intensity in gamma space.
    float color_correction[4] = {1.f, 1.f, 1.f, 1.f};
    if (ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID) {
        ArLightEstimate_getColorCorrection(_ar_session, ar_light_estimate, color_correction);
    }

    ArLightEstimate_destroy(ar_light_estimate);
    ar_light_estimate = nullptr;

    // -------------------- Render Andy objects. -------------------- //
    /*glm::mat4 model_mat(1.0f);
    // cycle through the colored anchors (contains the color of the object, and the anchor itself
    // which has the position/model matrix)
    for (const auto& colored_anchor : anchors_) {
        ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
        ArAnchor_getTrackingState(_ar_session, colored_anchor.anchor, &tracking_state);

        // Render object only if the tracking state is AR_TRACKING_STATE_TRACKING.
        if (tracking_state == AR_TRACKING_STATE_TRACKING) {
            // get the model matrix from the anchor
            util::GetTransformMatrixFromAnchor(*colored_anchor.anchor, _ar_session, &model_mat);
            // render that object
            andy_renderer_.Draw(projection_mat, view_mat, model_mat, color_correction, colored_anchor.color);
        }
    }*/

    // -------------------- Update and render planes. -------------------- //
    /*ArTrackableList* plane_list = nullptr;
    ArTrackableList_create(_ar_session, &plane_list);
    CHECK(plane_list != nullptr);

    ArTrackableType plane_tracked_type = AR_TRACKABLE_PLANE;
    ArSession_getAllTrackables(_ar_session, plane_tracked_type, plane_list);

    int32_t plane_list_size = 0;
    ArTrackableList_getSize(_ar_session, plane_list, &plane_list_size);
    plane_count_ = plane_list_size;

    for (int i = 0; i < plane_list_size; ++i) {
        ArTrackable* ar_trackable = nullptr;
        ArTrackableList_acquireItem(_ar_session, plane_list, i, &ar_trackable);
        ArPlane* ar_plane = ArAsPlane(ar_trackable);
        ArTrackingState out_tracking_state;
        ArTrackable_getTrackingState(_ar_session, ar_trackable, &out_tracking_state);

        ArPlane* subsume_plane;
        ArPlane_acquireSubsumedBy(_ar_session, ar_plane, &subsume_plane);
        if (subsume_plane != nullptr) {
            ArTrackable_release(ArAsTrackable(subsume_plane));
            continue;
        }

        if (ArTrackingState::AR_TRACKING_STATE_TRACKING != out_tracking_state) {
            continue;
        }

        ArTrackingState plane_tracking_state;
        ArTrackable_getTrackingState(_ar_session, ArAsTrackable(ar_plane), &plane_tracking_state);
        if (plane_tracking_state == AR_TRACKING_STATE_TRACKING) {

            const auto iter = plane_color_map_.find(ar_plane);
            glm::vec3 color;
            if (iter != plane_color_map_.end()) {
                color = iter->second;

                // If this is an already observed trackable release it so it doesn't
                // leave an additional reference dangling.
                ArTrackable_release(ar_trackable);
            } else {
                // The first plane is always white.
                if (!first_plane_has_been_found_) {
                    first_plane_has_been_found_ = true;
                    color = kWhite;
                } else {
                color = GetRandomPlaneColor();
                }
                plane_color_map_.insert({ar_plane, color});
            }
            plane_renderer_.Draw(projection_mat, view_mat, *_ar_session, *ar_plane, color);
        }
    }

    ArTrackableList_destroy(plane_list);
    plane_list = nullptr;*/

    // -------------------- Update and render point cloud. -------------------- //
    /*ArPointCloud* ar_point_cloud = nullptr;
    ArStatus point_cloud_status = ArFrame_acquirePointCloud(_ar_session, _ar_frame, &ar_point_cloud);
    if (point_cloud_status == AR_SUCCESS) {
        point_cloud_renderer_.Draw(projection_mat * view_mat, _ar_session, ar_point_cloud);
        ArPointCloud_release(ar_point_cloud);
    }*/
}

void ArApplication::onDrawFrame(GLuint textureId) {
    if(_ar_session == nullptr)
        return;

    ArSession_setCameraTextureName(_ar_session, textureId);
    // Update session to get current frame and render camera background.
    if (ArSession_update(_ar_session, _ar_frame) != AR_SUCCESS) {
        LOGE("OnDrawFrame ArSession_update error");
    }
    ArCamera* camera;
    ArFrame_acquireCamera(_ar_session, _ar_frame, &camera);

    glm::mat4 view_mat;
    glm::mat4 projection_mat;
    ArCamera_getViewMatrix(_ar_session, camera, glm::value_ptr(view_mat));
    ArCamera_getProjectionMatrix(_ar_session,camera, 0.1f, 100.0f, glm::value_ptr(projection_mat));

    ArPose* camera_pose = nullptr;
    ArPose_create(_ar_session, nullptr, &camera_pose);
    ArCamera_getPose(_ar_session, camera, camera_pose);
    float camera_pose_raw[7] = {0.f};
    ArPose_getPoseRaw(_ar_session, camera_pose, camera_pose_raw);

    ArTrackingState camera_tracking_state;
    ArCamera_getTrackingState(_ar_session, camera, &camera_tracking_state);
    ArCamera_release(camera);
}

void ArApplication::touchedPose(float x, float y, float* out_pose_raw){
    if (_ar_frame != nullptr && _ar_session != nullptr) {
        ArHitResultList *hit_result_list = nullptr;
        ArHitResultList_create(_ar_session, &hit_result_list);
        CHECK(hit_result_list);
        ArFrame_hitTest(_ar_session, _ar_frame, x, y, hit_result_list);

        int32_t hit_result_list_size = 0;
        ArHitResultList_getSize(_ar_session, hit_result_list, &hit_result_list_size);

        ArHitResult* ar_hit_result = nullptr;
        ArTrackableType trackable_type = AR_TRACKABLE_NOT_VALID;
        // search through the points in the hit_result_list
        for (int32_t i = 0; i < hit_result_list_size; ++i) {

            // get the item from the list
            ArHitResult *ar_hit = nullptr;
            ArHitResult_create(_ar_session, &ar_hit);
            ArHitResultList_getItem(_ar_session, hit_result_list, i, ar_hit);

            if (ar_hit == nullptr) {
                LOGE("ArApplication::OnTouched ArHitResultList_getItem error");
                return;
            }

            // figure out the type (if it is a trackable point or on a plane..)
            ArTrackable *ar_trackable = nullptr;
            ArHitResult_acquireTrackable(_ar_session, ar_hit, &ar_trackable);
            ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
            ArTrackable_getType(_ar_session, ar_trackable, &ar_trackable_type);

            if (ar_trackable_type != AR_TRACKABLE_NOT_VALID && ar_trackable_type != AR_TRACKABLE_AUGMENTED_IMAGE) {
                // gets the pose (model matrix) and checks if that is on the plane(isPoseInPolygon)
                ArPose *hit_pose = nullptr;
                ArPose_create(_ar_session, nullptr, &hit_pose);
                ArHitResult_getHitPose(_ar_session, ar_hit, hit_pose);
                // get the float list of pose values (last three are the x/y/z translations
                ArPose_getPoseRaw(_ar_session, hit_pose, out_pose_raw);
            }
        }
    }
}


void ArApplication::OnTouched(float x, float y) {

    if (_ar_frame != nullptr && _ar_session != nullptr) {
        ArHitResultList* hit_result_list = nullptr;
        ArHitResultList_create(_ar_session, &hit_result_list);
        CHECK(hit_result_list);
        ArFrame_hitTest(_ar_session, _ar_frame, x, y, hit_result_list);

        int32_t hit_result_list_size = 0;
        ArHitResultList_getSize(_ar_session, hit_result_list, &hit_result_list_size);

        // The hitTest method sorts the resulting list by distance from the camera,
        // increasing.  The first hit result will usually be the most relevant when
        // responding to user input.

        ArHitResult* ar_hit_result = nullptr;
        ArTrackableType trackable_type = AR_TRACKABLE_NOT_VALID;
        // search through the points in the hit_result_list
        for (int32_t i = 0; i < hit_result_list_size; ++i) {

            // get the item from the list
            ArHitResult* ar_hit = nullptr;
            ArHitResult_create(_ar_session, &ar_hit);
            ArHitResultList_getItem(_ar_session, hit_result_list, i, ar_hit);

            if (ar_hit == nullptr) {
                LOGE("ArApplication::OnTouched ArHitResultList_getItem error");
                return;
            }

            // figure out the type (if it is a trackable point or on a plane..)
            ArTrackable* ar_trackable = nullptr;
            ArHitResult_acquireTrackable(_ar_session, ar_hit, &ar_trackable);
            ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
            ArTrackable_getType(_ar_session, ar_trackable, &ar_trackable_type);

            // Creates an anchor if a plane or an oriented point was hit.
            // if on a plane
            if (AR_TRACKABLE_PLANE == ar_trackable_type) {

                // gets the pose (model matrix) and checks if that is on the plane(isPoseInPolygon)
                ArPose* hit_pose = nullptr;
                ArPose_create(_ar_session, nullptr, &hit_pose);
                ArHitResult_getHitPose(_ar_session, ar_hit, hit_pose);
                int32_t in_polygon = 0;
                ArPlane* ar_plane = ArAsPlane(ar_trackable);
                ArPlane_isPoseInPolygon(_ar_session, ar_plane, hit_pose, &in_polygon);

                // Use hit pose and camera pose to check if hittest is from the
                // back of the plane, if it is, no need to create the anchor.
                ArPose* camera_pose = nullptr;
                ArPose_create(_ar_session, nullptr, &camera_pose);
                ArCamera* ar_camera;
                ArFrame_acquireCamera(_ar_session, _ar_frame, &ar_camera);
                ArCamera_getPose(_ar_session, ar_camera, camera_pose);
                ArCamera_release(ar_camera);
                float normal_distance_to_plane = util::CalculateDistanceToPlane(*_ar_session, *hit_pose, *camera_pose);

                ArPose_destroy(hit_pose);
                ArPose_destroy(camera_pose);

                // if the distance to the plane is negative (behind the camera)
                // OR  the point was not in the plane ... don't draw continue on to the next point in the list
                if (!in_polygon || normal_distance_to_plane < 0) {
                    continue;
                }

                ar_hit_result = ar_hit;
                trackable_type = ar_trackable_type;
                break;
            }
            // if on a trackable point
            else if (AR_TRACKABLE_POINT == ar_trackable_type) {
                ArPoint* ar_point = ArAsPoint(ar_trackable);
                ArPointOrientationMode mode;
                ArPoint_getOrientationMode(_ar_session, ar_point, &mode);
                if (AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL == mode) {
                    ar_hit_result = ar_hit;
                    trackable_type = ar_trackable_type;
                    break;
                }
            }
        }

        // now that we have the point (on either a trackable point or on a plane)
        // make a new anchor
        if (ar_hit_result) {
            // Note that the application is responsible for releasing the anchor
            // pointer after using it. Call ArAnchor_release(anchor) to release.
            ArAnchor* anchor = nullptr;
            if (ArHitResult_acquireNewAnchor(_ar_session, ar_hit_result, &anchor) != AR_SUCCESS) {
                LOGE("ArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                return;
            }

            ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
            ArAnchor_getTrackingState(_ar_session, anchor, &tracking_state);
            if (tracking_state != AR_TRACKING_STATE_TRACKING) {
                ArAnchor_release(anchor);
                return;
            }
            // Don't render more than 20 droids
            if (anchors_.size() >= kMaxNumberOfAndroidsToRender) {
                ArAnchor_release(anchors_[0].anchor);
                anchors_.erase(anchors_.begin());
            }

            // Assign a color to the object for rendering based on the trackable type
            // this anchor attached to. For AR_TRACKABLE_POINT, it's blue color, and
            // for AR_TRACKABLE_PLANE, it's green color.
            ColoredAnchor colored_anchor;
            colored_anchor.anchor = anchor;
            switch (trackable_type) {
                case AR_TRACKABLE_POINT:
                    SetColor(66.0f, 133.0f, 244.0f, 255.0f, colored_anchor.color);
                    break;
                case AR_TRACKABLE_PLANE:
                    SetColor(139.0f, 195.0f, 74.0f, 255.0f, colored_anchor.color);
                    break;
                default:
                    SetColor(0.0f, 0.0f, 0.0f, 0.0f, colored_anchor.color);
                    break;
            }
            anchors_.push_back(colored_anchor);

        ArHitResult_destroy(ar_hit_result);
        ar_hit_result = nullptr;

        ArHitResultList_destroy(hit_result_list);
        hit_result_list = nullptr;
        }
    }
}

void ArApplication::SetColor(float r, float g, float b, float a, float* color4f) {
    *(color4f) = r;
    *(color4f + 1) = g;
    *(color4f + 2) = b;
    *(color4f + 3) = a;
}

float *ArApplication::updateBackgroundRender() {
    // UVs of the quad vertices (S, T)
    const GLfloat uvs[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,};
    int32_t geometry_changed = 0;
    ArFrame_getDisplayGeometryChanged(_ar_session, _ar_frame, &geometry_changed);
    if (geometry_changed != 0){
        ArFrame_transformDisplayUvCoords(_ar_session, _ar_frame, 8, uvs, transformed_camera_uvs);
        return transformed_camera_uvs;
    }
    return nullptr;
}

void ArApplication::estimateLight() {
    ArLightEstimate* ar_light_estimate;
    ArLightEstimateState ar_light_estimate_state;
    ArLightEstimate_create(_ar_session, &ar_light_estimate);

    ArFrame_getLightEstimate(_ar_session, _ar_frame, ar_light_estimate);
    ArLightEstimate_getState(_ar_session, ar_light_estimate, &ar_light_estimate_state);
    if(ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID){
        ArLightEstimate_getColorCorrection(_ar_session, ar_light_estimate, _light.color_correction);
        ArLightEstimate_getPixelIntensity(_ar_session, ar_light_estimate, &_light.intensity);
    }

    ArLightEstimate_destroy(ar_light_estimate);
}

