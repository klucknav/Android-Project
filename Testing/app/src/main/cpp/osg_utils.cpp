#include "osg_utils.h"

using namespace osg_utils;

void osg_utils::setupCamera(Camera *camera, GraphicsContext *context, int width, int height,
                            double fovy, double zNear, double zFar) {

    camera->setGraphicsContext(context);
    camera->setViewport(new Viewport(0,0,width, height));
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    camera->setProjectionMatrixAsPerspective(fovy, aspect, zNear, zFar);
}
osg::Program *osg_utils::createShaderProgram(const char *vertShader, const char *fragShader) {
    Shader *vShader = new Shader(Shader::VERTEX, vertShader);
    Shader *fShader = new Shader(Shader::FRAGMENT, fragShader);

    Program *program = new Program();
    program->addShader(vShader);
    program->addShader(fShader);
    return program;
}
osg::Program* osg_utils::createShaderProgram(const char* vertex_shader_file_name, const char* fragment_shader_file_name,
                                        AAssetManager* asset_manager){

    std::string VertexShaderContent;
    if (!util::LoadTextFileFromAssetManager(vertex_shader_file_name, asset_manager, &VertexShaderContent)) {
        LOGE("Failed to load file: %s", vertex_shader_file_name);
        return 0;
    }

    std::string FragmentShaderContent;
    if (!util::LoadTextFileFromAssetManager(fragment_shader_file_name, asset_manager, &FragmentShaderContent)) {
        LOGE("Failed to load file: %s", fragment_shader_file_name);
        return 0;
    }

    return createShaderProgram(VertexShaderContent.c_str(), FragmentShaderContent.c_str());
}
