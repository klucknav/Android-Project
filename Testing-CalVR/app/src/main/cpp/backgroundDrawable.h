#ifndef TESTING_OSG_BACKGROUND_H
#define TESTING_OSG_BACKGROUND_H

// OSG
#include <osg/BlendFunc>
#include <osg/MatrixTransform>

// GLES v2 and v3
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

// android
#include <android/asset_manager.h>

// other
#include "util.h"
#include "glDrawable.h"

class backgroundDrawable : public glDrawable {
private:
    GLuint _shader_program;
    GLuint _textureID;

    GLuint _attribVert;
    GLuint _attribUVs;

    GLuint _VAO;
    GLuint _VBO[2];

    const GLfloat _vertices[12] = {
            -1.0f, -1.0f, 0.0f, +1.0f, -1.0f, 0.0f,
            -1.0f, +1.0f, 0.0f, +1.0f, +1.0f, 0.0f,
    };
    // UVs of the quad vertices (S, T)
    const GLfloat _uvs[8] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

public:
    void init(AAssetManager* asset_manager);
    void updateUVs(float *new_uvs);
    void drawImplementation(osg::RenderInfo&) const;

    GLuint GetTextureId(){ return _textureID; }
    osg::ref_ptr<osg::Geode> createDrawableNode(){
        osg::ref_ptr<osg::Geode> glNode = glDrawable::createDrawableNode();
        glNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
        glNode->getOrCreateStateSet()->setRenderBinDetails(1, "RenderBin");
        return glNode.get();
    }
};


#endif //TESTING_OSG_BACKGROUND_H
