#pragma once

#include "config.h"
#include <framework/shader.h>
#include <glm/glm.hpp>

class Snow {
public:
    explicit Snow(const Config& config);
    ~Snow();
    Snow(const Snow&) = delete;
    Snow& operator=(const Snow&) = delete;

    void update(float deltaTime);
    void draw(const glm::mat4& viewProjection) const;

private:
    const Config& m_config;
    Shader m_shader;
    static constexpr GLsizei PARTICLE_COUNT = 2000;
    // Integrated wind and fall distances keep speed changes continuous.
    glm::dvec2 m_displacement{ 0.0 };
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};
