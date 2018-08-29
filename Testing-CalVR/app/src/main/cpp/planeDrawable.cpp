#include "planeDrawable.h"

void planeDrawable::_update_plane_vertices(const ArSession *arSession, const ArPlane *arPlane) {

    // clear previous data
    _vertices.clear();
    _triangles.clear();

    // get plane size
    int32_t polygonLength;
    ArPlane_getPolygonSize(arSession, arPlane, &polygonLength);
    if(polygonLength == 0){
        LOGE("PlaneDrawable: NO valid plane");
        return;
    }

    // get the vertices of the plane
    const int32_t verticesSize = polygonLength/2;
    std::vector<glm::vec2> rawVertices(verticesSize);
    ArPlane_getPolygon(arSession, arPlane, glm::value_ptr(rawVertices.front()));
    // fill to 3D
    for (int32_t i = 0; i < verticesSize; i++){
        _vertices.push_back(glm::vec3(rawVertices[i].x, rawVertices[i].y, 0.0f));
    }

    // get the Model matrix and normal for the plane
    ArPose * arPose;
    ArPose_create(arSession, nullptr, &arPose);
    ArPlane_getCenterPose(arSession, arPlane, arPose);
    ArPose_getMatrix(arSession, arPose, glm::value_ptr(_model_mat));
    _normal_vec = util::GetPlaneNormal(*arSession, *arPose);

    // Feather distance 0.2 meters.
    const float kFeatherLength = 0.2f;
    // Feather scale over the distance between plane center and vertices.
    const float kFeatherScale = 0.2f;

    // Fill vertex 4 to 7, with alpha set to 1.
    for (int32_t i = 0; i < verticesSize; ++i) {
        // Vector from plane center to current point.
        glm::vec2 v = rawVertices[i];
        const float scale = 1.0f - std::min((kFeatherLength / glm::length(v)), kFeatherScale);
        const glm::vec2 result_v = scale * v;

        _vertices.push_back(glm::vec3(result_v.x, result_v.y, 1.0f));
    }

    // generate Triangles
    const int32_t vertLen = _vertices.size();
    const int32_t halfVertLen = vertLen / 2;

    // Generate triangle (4, 5, 6) and (4, 6, 7).
    for (int i = halfVertLen + 1; i < vertLen - 1; ++i) {
        _triangles.push_back(halfVertLen);
        _triangles.push_back(i);
        _triangles.push_back(i + 1);
    }

    // Generate triangle (0, 1, 4), (4, 1, 5), (5, 1, 2), (5, 2, 6),
    // (6, 2, 3), (6, 3, 7), (7, 3, 0), (7, 0, 4)
    for (int i = 0; i < halfVertLen; ++i) {
        _triangles.push_back(i);
        _triangles.push_back((i + 1) % halfVertLen);
        _triangles.push_back(i + halfVertLen);

        _triangles.push_back(i + halfVertLen);
        _triangles.push_back((i + 1) % halfVertLen);
        _triangles.push_back((i + halfVertLen + 1) % halfVertLen + halfVertLen);
    }
}

void planeDrawable::init(AAssetManager *manager) {

    _shader_program = util::CreateProgram("shaders/plane.vert", "shaders/plane.frag", manager);
    if(!_shader_program)
        LOGE("PlaneDrawable: Failed to create shader program");

    // initialize the attributes & uniforms
    _attrib_vertices = glGetAttribLocation(_shader_program, "vPosition");
    _uniform_model_mat = glGetUniformLocation(_shader_program, "uModel");
    _uniform_normal_vec = glGetUniformLocation(_shader_program, "uNormal");
    _uniform_mvp_mat = glGetUniformLocation(_shader_program, "uMVP");
    _uniform_tex_sampler = glGetUniformLocation(_shader_program, "uTexture");
    _uniform_color = glGetUniformLocation(_shader_program, "uColor");

    // initialize the texture
    glGenTextures(1, &_texture_id);
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load texture
    if(!util::LoadPngFromAssetManager(GL_TEXTURE_2D, "models/trigrid.png")) {
        LOGE("PlaneDrawable: Failed to load png image");
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // generate VAO and bind
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);

    // generate VBO and bind
    glGenBuffers(1, &_VBO);

    // send data
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAX_PLANE_VERTICES * 3, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(_attrib_vertices);
    glVertexAttribPointer(_attrib_vertices, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // generate EBO
    glGenBuffers(1, &_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * MAX_PLANE_VERTICES*5, nullptr, GL_STATIC_DRAW);

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void planeDrawable::updateOnFrame(const ArSession *arSession, const ArPlane *arPlane,
                                  const glm::mat4 &projMat, const glm::mat4 &viewMat,
                                  const glm::vec3 &color) {

    LOGI("===== PlaneDrawable: update onFrame =====");
    _projMat = projMat;
    _viewMat = viewMat;

    // get the plane vertices from AR-Core
    _update_plane_vertices(arSession, arPlane);

//    LOGE("PlaneDrawable: DRAW PLANE: %d", _vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*_vertices.size()*3, _vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLushort) * _triangles.size(), _triangles.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(_shader_program);
    glUniform3f(_uniform_color, color.x, color.y, color.z);
    glUseProgram(0);
}

void planeDrawable::drawImplementation(osg::RenderInfo &) const {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(_shader_program);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(_uniform_tex_sampler, 0);
    glBindTexture(GL_TEXTURE_2D, _texture_id);

    glUniformMatrix4fv(_uniform_mvp_mat, 1, GL_FALSE, glm::value_ptr(_projMat * _viewMat * _model_mat));
    glUniform3f(_uniform_normal_vec, _normal_vec.x, _normal_vec.y, _normal_vec.z);
    glUniformMatrix4fv(_uniform_model_mat, 1, GL_FALSE, glm::value_ptr(_model_mat));

    glBindVertexArray(_VAO);

    glDrawElements(GL_TRIANGLES, _triangles.size(), GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);

    glUseProgram(0);
//    util::CheckGlError("Draw planes");        // crashes here
}
