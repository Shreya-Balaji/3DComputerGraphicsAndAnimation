#pragma once

#include "model.h"
#include "config.h"
#include "light.h"
#include "mesh.h"
#include <memory>

class CableCar {
public:
    explicit CableCar(const Config& config);

glm::vec3 position() const;
glm::vec3 position(float terrainOffset) const;

void update(float deltaTime, float terrainOffset);

void draw(
    const glm::mat4 viewProjection,
    const glm::vec3 cameraPosition,
    const SkyLight& light,
    bool usePbr,
    bool isMinimap = false
) const;

void drawDepth(const glm::mat4& lightCamera) const;

private:
    static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.3f);
    void createCable();
    const Config& m_config;
    Model m_model;
    std::unique_ptr<GPUMesh> m_cableMesh;
    Shader m_cableCarShader;
    Shader m_pbrShader;
        Shader m_depthShader;
    float m_wheelAngle = 0.0f;
    float m_terrainOffset = 0.0f;
};
