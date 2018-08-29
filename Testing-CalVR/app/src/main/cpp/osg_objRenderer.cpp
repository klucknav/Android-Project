#include "osg_objRenderer.h"

#include <osg/Texture2D>

osg::ref_ptr<osg::Geode> osg_objRenderer::createNode(AAssetManager *manager,
                         const char *obj_file_name, const char *png_file_name) {

    LOGI("===== ANDY: Create Node =====");
    // initialize osg
    _geometry = new osg::Geometry();
    _geode = new osg::Geode();
    _geode->addDrawable(_geometry.get());

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> uvs = new osg::Vec2Array();

    // initialize vectors to get information from obj file
    std::vector<GLfloat> _vertices;
    std::vector<GLfloat> _normals;
    std::vector<GLfloat> _uvs;
    std::vector<GLushort> _indices;


    util::LoadObjFile(obj_file_name, manager, &_vertices, &_normals, &_uvs, &_indices);
    LOGI("===== ANDY: Create Node - loaded the obj file =====");

    // send info to the osg containers
    for (int i = 0; i < _uvs.size(); i++) {
        vertices->push_back(osg::Vec3f(_vertices[3 * i], _vertices[3 * i + 1], _vertices[3 * i + 2]));
        normals->push_back(osg::Vec3f(_normals[3 * i], _normals[3 * i + 1], _normals[3 * i + 2]));
        uvs->push_back(osg::Vec2f(_uvs[2 * i], _uvs[2 * i + 1]));
    }


    // create the geometry
    _geometry->setVertexArray(vertices.get());
    _geometry->setNormalArray(normals.get());
    _geometry->setTexCoordArray(0, uvs.get());
    _geometry->addPrimitiveSet(new osg::DrawElementsUShort(GL_TRIANGLES, _indices.size(), _indices.data()));
    _geometry->setUseVertexBufferObjects(true);
    _geometry->setUseDisplayList(false);


    // initialize the texture
    LOGI("===== ANDY: Create Node - initializing the texture =====");
    osg::Texture2D* objTexture = new Texture2D();
    objTexture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR);
    objTexture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
    objTexture->setWrap(Texture::WRAP_T, Texture::REPEAT);
    objTexture->setWrap(Texture::WRAP_S, Texture::REPEAT);
    LOGI("===== ANDY: Create Node - texture initialized =====");

    if (!util::LoadPngFromAssetManager(GL_TEXTURE_2D, png_file_name)) {
        LOGE("Could not load png texture for planes.");
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);


    // get the information for the shader
    osg::Uniform* _uniform_sampler = new osg::Uniform(osg::Uniform::SAMPLER_2D, "uTexture");
    osg::Uniform * uniform_material = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "uMaterialParams");
    _uniform_sampler->set(0);
    uniform_material->set(Vec4f(_shader_params.ambient, _shader_params.diffuse,
                                _shader_params.specular, _shader_params.specular_alpha));

    _uniform_mvp = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "uMVP");
    _uniform_mv = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "uMV");
    _uniform_light = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "uLightingParams");
    _uniform_color_correct = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "uColorCorrection");

    // create the program and send the info to the shader
    Program * program = osg_utils::createShaderProgram("shaders/osgObject.vert", "shaders/osgObject.frag", manager);
    osg::StateSet* stateset = _geode->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0, objTexture, osg::StateAttribute::ON);
    stateset->addUniform(_uniform_sampler);
    stateset->addUniform(_uniform_mvp);
    stateset->addUniform(uniform_material);
    stateset->addUniform(_uniform_mv);
    stateset->addUniform(_uniform_light);
    stateset->addUniform(_uniform_color_correct);
    stateset->setAttributeAndModes(program);

    return _geode.get();
}

void
osg_objRenderer::Draw(const glm::mat4 &projMat, const glm::mat4 &viewMat, const glm::mat4 &modelMat,
                      const float *color_correction, float light_intensity) {

    glm::mat4 mv = viewMat * modelMat;
    glm::vec4 v_light = glm::normalize(mv * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));

    _uniform_mv->set(*(new RefMatrixf(glm::value_ptr(mv))));
    _uniform_mvp->set(*(new RefMatrixf(glm::value_ptr(projMat* mv))));
    _uniform_light->set(Vec4f(v_light[0], v_light[1], v_light[2], light_intensity));
    _uniform_color_correct->set(Vec4f(color_correction[0], color_correction[1],
                                      color_correction[2], color_correction[3]));
}
