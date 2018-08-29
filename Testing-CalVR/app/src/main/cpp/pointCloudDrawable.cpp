#include "pointCloudDrawable.h"
#include <stack>
#include <GLES3/gl3.h>

void pointCloudDrawable::init(AAssetManager *manager) {

    _shader_program = util::CreateProgram("shaders/pointDrawable.vert", "shaders/pointDrawable.frag", manager);

    _attrib_vertices = glGetAttribLocation(_shader_program,"vPosition");
    _uniform_arMVP_mat =  glGetUniformLocation(_shader_program, "uarMVP");

    //Generate VAO and bind
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);

    //Generate VBO and bind
    glGenBuffers(1, &_VBO);

    //dynamic feed data
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAX_POINTS * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(_attrib_vertices);
    glVertexAttribPointer(_attrib_vertices, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(_shader_program);
    glUniform4fv(glGetUniformLocation(_shader_program, "uColor"), 1, glm::value_ptr(_default_color));
    glUniform1f(glGetUniformLocation(_shader_program, "uPointSize"), _default_size);
    glUseProgram(0);
}

void pointCloudDrawable::drawImplementation(osg::RenderInfo &) const {
    glUseProgram(_shader_program);
    glUniformMatrix4fv(_uniform_arMVP_mat, 1, GL_FALSE, glm::value_ptr(_ar_mvp));

    glBindVertexArray(_VAO);
    glDrawArrays(GL_POINTS, 0, _point_num);
    glBindVertexArray(0);

    glUseProgram(0);
//    util::CheckGlError("Draw pointCloud");    // crashes here
}
