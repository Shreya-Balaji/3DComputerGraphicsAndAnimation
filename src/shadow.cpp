#include "shadow.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <stdexcept>
#include <glm/glm.hpp>

ShadowMap::ShadowMap()
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SIZE, SIZE, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!complete)
        throw std::runtime_error("ShadowMap: framebuffer is not complete");
}

ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &m_framebuffer);
    glDeleteTextures(1, &m_texture);
}

void ShadowMap::begin()
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_previousFramebuffer);
    glGetIntegerv(GL_VIEWPORT, m_previousViewport);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, SIZE, SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(m_previousFramebuffer));
    glViewport(m_previousViewport[0], m_previousViewport[1],
               m_previousViewport[2], m_previousViewport[3]);
}

void ShadowMap::bind(int textureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glActiveTexture(GL_TEXTURE0); 
}

glm::mat4 computeLightSpace(const glm::vec3& shadowLightDir, const glm::vec3& centre)
{
    glm::vec3 toLight = computeShadowToLight(shadowLightDir);
    const float minElev = glm::radians(40.0f);
    if(toLight.y < sin(minElev)){
        glm::vec2 horizontal = normalize(glm::vec2(toLight.x, toLight.z));
        toLight = glm::vec3(horizontal.x * cos(minElev), sin(minElev), horizontal.y * cos(minElev));

    }
    const glm::vec3 lightPos = centre + toLight * 40.0f;
    const glm::vec3 up = std::abs(toLight.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::mat4 lightView = glm::lookAt(lightPos, centre, up);
    const glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 100.0f);
    return lightProjection * lightView;
}
glm::vec3 computeShadowToLight(const glm::vec3& shadowLightDir)
{
    glm::vec3 toLight = -shadowLightDir;
    const float minElev = glm::radians(40.0f);
    if (toLight.y < std::sin(minElev))
    {
        glm::vec2 horiz = glm::normalize(glm::vec2(toLight.x, toLight.z));
        toLight = glm::vec3(horiz.x * std::cos(minElev), std::sin(minElev), horiz.y * std::cos(minElev));
    }
    return toLight;
}