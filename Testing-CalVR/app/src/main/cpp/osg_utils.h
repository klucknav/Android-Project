#ifndef TESTING_OSG_UTIL_H
#define TESTING_OSG_UTIL_H

#include <osg/Camera>
#include <osgDB/ReadFile>

#include "util.h"

// Initialize OSG plugins when OpenSceneGraph is built as a static library.
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(tiff)
//USE_OSGPLUGIN(png)
//USE_OSGPLUGIN(jpeg)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

using namespace osg;

namespace osg_utils {
    void setupCamera(Camera *camera, GraphicsContext *context, int width, int height,
                     double fovy = 30, double zNear = 1, double zFar = 1000);

    osg::Program* createShaderProgram(const char *vertShader, const char *fragShader);

    osg::Program* createShaderProgram(const char* vertex_shader_file_name, const char* fragment_shader_file_name,
                                 AAssetManager* asset_manager);
}

#endif //TESTING_OSG_UTIL_H
