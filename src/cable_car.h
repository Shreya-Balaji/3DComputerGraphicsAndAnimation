#pragma once

#include "model.h"
#include "config.h"
#include "light.h"

class CableCar {
    public:
        explicit CableCar(const Config& config);
        glm::vec3 position() const;
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition, const SkyLight& light, bool usePbr) const;
        void drawDepth(const glm::mat4& lightCamera) const;
        void update(float deltaTime);
    private:
        static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.3f);

        const Config& m_config;
        Model m_model;
        Shader m_pbrShader;
        Shader m_cableCarShader;
        Shader m_depthShader;
        float m_wheelAngle = 0.0f;
};
