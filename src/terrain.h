#pragma once

#include "config.h"
#include "mesh.h"
#include <memory>
#include "light.h"

class Terrain {
public:
    Terrain(const Config& config, const glm::vec3& position);
    void update(float deltaTime);
    void draw(const glm::mat4& viewProjection, const glm::vec3& viewPosition, const SkyLight& light) const;
    void drawReflection(const glm::mat4& viewProjection, const SkyLight& light) const;

private:
    const Config& m_config;
    const glm::vec3& m_position;
    Shader m_shader;
    std::unique_ptr<GPUMesh> m_mesh;
    Shader m_reflectionShader;
    std::unique_ptr<GPUMesh> m_reflectionMesh;
    double m_scroll = 0.0;
};
