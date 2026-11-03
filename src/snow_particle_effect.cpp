#include "snow_particle_effect.h"

#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <random>
#include <string>

SnowParticleEffect::SnowParticleEffect(const Config& config, const glm::vec3& position)
    : m_config(config)
    , m_position(position)
{
    ShaderBuilder builder;
    builder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/snow.vert");
    builder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/snow.frag");
    m_shader = builder.build();

    std::mt19937 random(std::random_device{}());
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);

    // a particle consists of 3 fields for its position and one for size
    std::array<glm::vec4, PARTICLE_COUNT> particles;
    // spawn particles in random positions inside the box defined by EXTENT
    for (auto& particle : particles) {
        particle = glm::vec4(
            unit(random) * EXTENT.x,
            unit(random) * EXTENT.y,
            unit(random) * EXTENT.z,
            0.7f + unit(random) * 0.6f);
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

SnowParticleEffect::~SnowParticleEffect()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void SnowParticleEffect::update(float deltaTime)
{
    if (!m_config.snowEnabled)
        return;

    m_displacement.x += static_cast<double>(m_config.snowWind) * deltaTime;
    m_displacement.y += static_cast<double>(m_config.snowFallSpeed) * deltaTime;
}

void SnowParticleEffect::draw(const glm::mat4& viewProjection) const
{
    if (!m_config.snowEnabled)
        return;

    // the origin is at the center of the box
    const glm::vec3 minimum = m_position - EXTENT * 0.5f;

    const GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    const GLboolean pointSizeEnabled = glIsEnabled(GL_PROGRAM_POINT_SIZE);
    GLboolean depthWrite;
    GLint sourceRGB, destinationRGB, sourceAlpha, destinationAlpha;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glGetIntegerv(GL_BLEND_SRC_RGB, &sourceRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &destinationRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &sourceAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &destinationAlpha);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_FALSE);

    m_shader.bind();
    glUniformMatrix4fv(m_shader.getUniformLocation("viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform1f(m_shader.getUniformLocation("snowSize"), m_config.snowSize);
    glUniform2f(m_shader.getUniformLocation("displacement"), static_cast<float>(m_displacement.x), static_cast<float>(m_displacement.y));
    glUniform3fv(m_shader.getUniformLocation("minimum"), 1, glm::value_ptr(minimum));
    glUniform3fv(m_shader.getUniformLocation("extent"), 1, glm::value_ptr(EXTENT));
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
    glBindVertexArray(0);

    glDepthMask(depthWrite);
    glBlendFuncSeparate(sourceRGB, destinationRGB, sourceAlpha, destinationAlpha);
    if (!blendEnabled)
        glDisable(GL_BLEND);
    if (!pointSizeEnabled)
        glDisable(GL_PROGRAM_POINT_SIZE);
}
