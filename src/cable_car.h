#pragma once

#include "model.h"
#include "config.h"
#include "light.h"
#include "mesh.h"
#include <memory>
#include <vector>

class CableCar {
public:
    explicit CableCar(const Config& config);

glm::vec3 position() const;

void update(float deltaTime);

    void draw(const glm::mat4& viewProjection, const glm::vec3& cameraPosition, const SkyLight& light) const;
    void drawMinimap(const glm::mat4& viewProjection, const glm::vec3& cameraPosition, const SkyLight& light) const;

    // Draw after the opaque scene so the glass blends with its background.
    void drawWindows(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                     const SkyLight& light) const;

void drawDepth(const glm::mat4& lightCamera) const;

private:
    static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.17f);
    void createCable();
    void drawCables(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                    const SkyLight& light, const glm::mat4& cableTransform) const;
    void bindLighting(const Shader& shader, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPosition, const SkyLight& light) const;
    void drawBody(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                  const SkyLight& light) const;
    const Config& m_config;
    Model m_model;
    std::vector<unsigned int> m_windowMeshes;
    std::vector<unsigned int> m_bodyMeshes;
    std::unique_ptr<GPUMesh> m_cableMesh;
    Shader m_cableCarShader;
    Shader m_pbrShader;
    Shader m_windowShader;
        Shader m_depthShader;
    float m_wheelAngle = 0.0f;
};
