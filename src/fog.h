#pragma once

#include "config.h"
#include <framework/shader.h>
#include <glm/glm.hpp>

class Fog {
public:
    explicit Fog(const Config& config);
    ~Fog();
    Fog(const Fog&) = delete;
    Fog& operator=(const Fog&) = delete;

    void begin(int width, int height);
    void draw(const glm::mat4& projection) const;

private:
    const Config& m_config;
    Shader m_shader;
    GLuint m_framebuffer = 0;
    GLuint m_colorTexture = 0;
    GLuint m_depthTexture = 0;
    GLuint m_vao = 0;
    int m_width = 0;
    int m_height = 0;
};
