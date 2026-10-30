#pragma once

#include "model.h"
#include "config.h"

class CableCar {
    public:
        explicit CableCar(const Config& config);
        glm::vec3 position() const;
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition) const;
        void update(float deltaTime);
    private:
        static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.3f);

        const Config& m_config;
        Model m_model;
        Shader m_cableCarShader;
        float m_wheelAngle = 0.0f;
};
