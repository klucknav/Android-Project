#include "backgroundDrawable.h"


void backgroundDrawable::init(AAssetManager* asset_manager) {

    _shader_program = util::CreateProgram("shaders/screenquad.vert", "shaders/backgroundText.frag", asset_manager);

    if (!_shader_program) {
        LOGE("Could not create program.");
    }

    _attribVert = glGetAttribLocation(_shader_program, "a_Position");
    _attribUVs = glGetAttribLocation(_shader_program, "a_TexCoord");

    glGenTextures(1, &_textureID);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureID);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);

    glGenBuffers(2, _VBO);

    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, _VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, _vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(_attribVert);
    glVertexAttribPointer(_attribVert, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    // texture coord
    glBindBuffer(GL_ARRAY_BUFFER, _VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(_attribUVs);
    glVertexAttribPointer(_attribUVs, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(_shader_program);
    glUniform1i(glGetUniformLocation(_shader_program, "uTexture"), 0);
    glUseProgram(0);
}

void backgroundDrawable::updateUVs(float *new_uvs) {
    glBindBuffer(GL_ARRAY_BUFFER, _VBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 8* sizeof(float), new_uvs);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void backgroundDrawable::drawImplementation(osg::RenderInfo&) const {

    glUseProgram(_shader_program);

    // screen quad has arbitrary depth, so should be drawn first.
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // activate texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureID);

    // bind and draw
    glBindVertexArray(_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

    // Restore the depth state for further drawing
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}
