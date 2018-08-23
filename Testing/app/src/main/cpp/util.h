#ifndef TESTING_UTIL_H_
#define TESTING_UTIL_H_

// GLES v2 and v3
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

// android and c++
#include <android/asset_manager.h>
#include <android/log.h>
#include <errno.h>
#include <jni.h>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ar_example_c", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ar_example_c", __VA_ARGS__)


#ifndef CHECK
#define CHECK(condition)                                                   \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    abort();                                                               \
  }
#endif  // CHECK


// Utilities for C hello AR project.
namespace util {

// Provides a scoped allocated instance of Anchor.
// Can be treated as an ArAnchor*.
class ScopedArPose {
public:
    explicit ScopedArPose(const ArSession* session) {
        ArPose_create(session, nullptr, &pose_);
    }
    ~ScopedArPose() { ArPose_destroy(pose_); }
    ArPose* GetArPose() { return pose_; }
    // Delete copy constructors.
    ScopedArPose(const ScopedArPose&) = delete;
    void operator=(const ScopedArPose&) = delete;

private:
    ArPose* pose_;
};

// Check GL error, and abort if an error is encountered.
//
// @param operation, the name of the GL function call.
void CheckGlError(const char* operation);

// Create a shader program ID.
//
// @param asset_manager, AAssetManager pointer.
// @param vertex_shader_file_name, the vertex shader source file.
// @param fragment_shader_file_name, the fragment shader source file.
// @return a non-zero value if the shader is created successfully, otherwise 0.
GLuint CreateProgram(const char* vertex_shader_file_name,
                     const char* fragment_shader_file_name,
                     AAssetManager* asset_manager);

// Load a text file from assets folder.
//
// @param asset_manager, AAssetManager pointer.
// @param file_name, path to the file, relative to the assets folder.
// @param out_string, output string.
// @return true if the file is loaded correctly, otherwise false.
bool LoadTextFileFromAssetManager(const char* file_name,
                                  AAssetManager* asset_manager,
                                  std::string* out_file_text_string);

// Load png file from assets folder and then assign it to the OpenGL target.
// This method must be called from the renderer thread since it will result in
// OpenGL calls to assign the image to the texture target.
//
// @param target, openGL texture target to load the image into.
// @param path, path to the file, relative to the assets folder.
// @return true if png is loaded correctly, otherwise false.
bool LoadPngFromAssetManager(int target, const std::string& path);

// Load obj file from assets folder from the app.
//
// @param asset_manager, AAssetManager pointer.
// @param file_name, name of the obj file.
// @param out_vertices, output vertices.
// @param out_normals, output normals.
// @param out_uv, output texture UV coordinates.
// @param out_indices, output triangle indices.
// @return true if obj is loaded correctly, otherwise false.
bool LoadObjFile(const std::string& file_name, AAssetManager* asset_manager,
                 std::vector<GLfloat>* out_vertices, std::vector<GLfloat>* out_normals,
                 std::vector<GLfloat>* out_uv, std::vector<GLushort>* out_indices);

// Format and output the matrix to logcat file.
// Note that this function output matrix in row major.
void Log4x4Matrix(const float raw_matrix[16]);

// Get transformation matrix from ArAnchor.
void GetTransformMatrixFromAnchor(const ArAnchor& ar_anchor, ArSession* ar_session, glm::mat4* out_model_mat);

// Get the plane's normal from center pose.
glm::vec3 GetPlaneNormal(const ArSession& ar_session, const ArPose& plane_pose);

// Calculate the normal distance to plane from cameraPose, the given planePose
// should have y axis parallel to plane's normal, for example plane's center
// pose or hit test pose.
float CalculateDistanceToPlane(const ArSession& ar_session, const ArPose& plane_pose, const ArPose& camera_pose);

}  // namespace util

#endif  // TESTING_UTIL_H_
