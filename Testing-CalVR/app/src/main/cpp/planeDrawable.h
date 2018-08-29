#ifndef TESTING_PLANEDRAWABLE_H
#define TESTING_PLANEDRAWABLE_H

#include "glDrawable.h"
#include "util.h"

#define MAX_PLANE_VERTICES 100
#define BUFFER_OFFSET(i) ((GLushort *)NULL + (i))

class planeDrawable : public glDrawable{
private:
    std::vector<glm::vec3> _vertices;
    std::vector<GLushort> _triangles;
    glm::mat4 _model_mat = glm::mat4(1.0f);
    glm::vec3 _normal_vec = glm::vec3(.0f);

    GLuint _texture_id;
    GLuint _shader_program;

    GLint _attrib_vertices;
    GLint _uniform_mvp_mat;
    GLint _uniform_tex_sampler;
    GLint _uniform_normal_vec;
    GLint _uniform_model_mat;
    GLint _uniform_color;

    GLuint _VAO;
    GLuint _VBO;
    GLuint _EBO;

    glm::mat4 _projMat;
    glm::mat4 _viewMat;
    void _update_plane_vertices(const ArSession * arSession, const ArPlane * arPlane);

public:
    void updateOnFrame(const ArSession * arSession, const ArPlane * arPlane,
                       const glm::mat4 & projMat, const  glm::mat4 & viewMat,
                       const  glm::vec3 & color);
    void init(AAssetManager * manager);
    void drawImplementation(osg::RenderInfo&) const;
};

#endif //TESTING_PLANEDRAWABLE_H
