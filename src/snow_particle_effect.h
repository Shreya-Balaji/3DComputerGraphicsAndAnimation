#pragma once

#include "config.h"
#include <framework/shader.h>
#include <glm/glm.hpp>

class SnowParticleEffect {
public:
    SnowParticleEffect(const Config& config, const glm::vec3& position);
    ~SnowParticleEffect();
    SnowParticleEffect(const SnowParticleEffect&) = delete;
    SnowParticleEffect& operator=(const SnowParticleEffect&) = delete;

    void update(float deltaTime);
    void draw(const glm::mat4& viewProjection) const;

private:
    const Config& m_config;
    const glm::vec3& m_position;
    Shader m_shader;
    static constexpr GLsizei PARTICLE_COUNT = 2000;
    glm::dvec2 m_displacement{ 0.0 };
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};
