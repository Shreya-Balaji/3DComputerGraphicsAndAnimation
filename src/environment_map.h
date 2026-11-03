#pragma once

#include <framework/shader.h>
#include <glm/glm.hpp>
#include <functional>

class EnvironmentMap {
public:
    static constexpr int SIZE = 256;
    static constexpr float MAX_LOD = 8.0f;
    using DrawScene = std::function<void(const glm::mat4&, const glm::mat4&, const glm::vec3&)>;

    EnvironmentMap();
    ~EnvironmentMap();
    EnvironmentMap(const EnvironmentMap&) = delete;
    EnvironmentMap& operator=(const EnvironmentMap&) = delete;

    void capture(const glm::vec3& position, const DrawScene& drawScene) const;
    void bind(int textureUnit) const;

private:
    GLuint m_cubemap = 0;
    GLuint m_framebuffer = 0;
    GLuint m_depthBuffer = 0;
};