#include "fog.h"

#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <string>

Fog::Fog(const Config& config)
    : m_config(config)
{
    ShaderBuilder builder;
    builder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/fog.vert");
    builder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/fog.frag");
    m_shader = builder.build();

    glGenFramebuffers(1, &m_framebuffer);
    glGenTextures(1, &m_colorTexture);
    glGenTextures(1, &m_depthTexture);
    glGenVertexArrays(1, &m_vao);
    for (GLuint texture : {m_colorTexture, m_depthTexture}) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Fog::~Fog()
{
    glDeleteFramebuffers(1, &m_framebuffer);
    glDeleteTextures(1, &m_colorTexture);
    glDeleteTextures(1, &m_depthTexture);
    glDeleteVertexArrays(1, &m_vao);
}

void Fog::begin(int width, int height)
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Fog framebuffer dimensions must be positive");

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    if (width != m_width || height != m_height) {
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Could not initialize fog framebuffer");
        m_width = width;
        m_height = height;
    }
    glViewport(0, 0, width, height);
}

void Fog::draw(const glm::mat4& projection, const glm::vec3& skyColor) const
{
    // Keep the scene depth for snow, which is rendered after the first fog pass
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);

    const GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blend = glIsEnabled(GL_BLEND);
    GLboolean depthWrite;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);

    m_shader.bind();
    const glm::mat4 inverseProjection = glm::inverse(projection);
    glUniformMatrix4fv(m_shader.getUniformLocation("inverseProjection"), 1, GL_FALSE, glm::value_ptr(inverseProjection));
    glUniform3fv(m_shader.getUniformLocation("fogColor"), 1, glm::value_ptr(skyColor));
    glUniform1i(m_shader.getUniformLocation("fogEnabled"), m_config.fogEnabled);
    glUniform1f(m_shader.getUniformLocation("fogStart"), m_config.fogStart);
    glUniform1f(m_shader.getUniformLocation("fogEnd"), m_config.fogEnd);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glUniform1i(m_shader.getUniformLocation("sceneColor"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glUniform1i(m_shader.getUniformLocation("sceneDepth"), 1);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDepthMask(depthWrite);
    if (depthTest)
        glEnable(GL_DEPTH_TEST);
    if (blend)
        glEnable(GL_BLEND);
}
