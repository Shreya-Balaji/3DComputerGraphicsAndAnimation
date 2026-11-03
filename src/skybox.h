#pragma once

#include <framework/shader.h>
#include <glm/glm.hpp>
#include "config.h"
#include "light.h"

class Skybox {
public:
    Skybox();
    ~Skybox();
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    void draw(const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& cameraPosition, const SkyLight& light,
              const glm::vec3& horizonColor) const;

private:
    Shader m_shader;
    GLuint m_vao = 0;
};