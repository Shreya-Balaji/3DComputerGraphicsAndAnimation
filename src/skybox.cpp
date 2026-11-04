#include "skybox.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>

Skybox::Skybox()
{
    const std::string root = PROJECT_ROOT;
    ShaderBuilder builder;
    builder.addStage(GL_VERTEX_SHADER, root + "/shaders/sky.vert");
    builder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/sky.frag");
    m_shader = builder.build();

    glGenVertexArrays(1, &m_vao);
}

Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &m_vao);
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection,
                  const glm::vec3& cameraPosition, const SkyLight& light,
                  const glm::vec3& horizonColor) const
{
    const glm::mat4 invViewProj = glm::inverse(projection * view);
    const glm::vec3 toSun = -light.sunDir;
    const glm::vec3 toMoon = -light.moonDir;

    m_shader.bind();
    glUniformMatrix4fv(m_shader.getUniformLocation("invViewProj"), 1, GL_FALSE, glm::value_ptr(invViewProj));
    glUniform3fv(m_shader.getUniformLocation("cameraPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(m_shader.getUniformLocation("toSun"), 1, glm::value_ptr(toSun));
    glUniform3fv(m_shader.getUniformLocation("toMoon"), 1, glm::value_ptr(toMoon));
    glUniform3fv(m_shader.getUniformLocation("horizonColor"), 1, glm::value_ptr(horizonColor));
    glUniform3fv(m_shader.getUniformLocation("sunColor"), 1, glm::value_ptr(light.sunColor));
    glUniform3fv(m_shader.getUniformLocation("moonColor"), 1, glm::value_ptr(light.moonColor));

    const GLboolean useDepthTest = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    if (useDepthTest)
        glEnable(GL_DEPTH_TEST);
}