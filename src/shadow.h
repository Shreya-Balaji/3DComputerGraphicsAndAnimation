#pragma once

#include <glm/glm.hpp>
#include "framework/shader.h"

class ShadowMap {
public:
    static constexpr int SIZE = 4096;

    ShadowMap();
    ~ShadowMap();
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    void begin();                
    void end();                   
    void bind(int textureUnit) const;

private:
    GLuint m_texture = 0;
    GLuint m_framebuffer = 0;
    GLint m_previousFramebuffer = 0;
    GLint m_previousViewport[4] = {0, 0, 0, 0};
};
glm::mat4 computeLightSpace(const glm::vec3& shadowLightDir, const glm::vec3& centre);
glm::vec3 computeShadowToLight(const glm::vec3& shadowLightDir);