#ifndef TESTING_OSG_OBJRENDERER_H
#define TESTING_OSG_OBJRENDERER_H

#include "osg_utils.h"

typedef struct{
    float ambient = 0.0f;
    float diffuse = 2.0f;
    float specular = 0.5f;
    float specular_alpha = 6.0f;
} objParams;

class osg_objRenderer{
private:
    objParams _shader_params;
    osg::ref_ptr<osg::Geometry> _geometry;
    osg::ref_ptr<osg::Geode> _geode;

    osg::Uniform *_uniform_mvp;
    osg::Uniform *_uniform_mv;
    osg::Uniform *_uniform_light;
    osg::Uniform *_uniform_color_correct;

public:
    osg::ref_ptr<osg::Geode> createNode(AAssetManager * manager, const char* obj_file_name,
                                        const char* png_file_name);
    void Draw(const glm::mat4 & projMat, const glm::mat4 & viewMat, const glm::mat4 & modelMat,
              const float * color_correction, float light_intensity);
};

#endif //TESTING_OSG_OBJRENDERER_H
