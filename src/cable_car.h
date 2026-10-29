#pragma once

#include "model.h"

class CableCar {
    public:
        CableCar();
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition) const;
        void setDoorSlideFactor(float slideFactor);
        void updateWheels(float deltaTime, float speedDegreesPerSecond);
        void setWorldTransform(
            const glm::vec3& position,
            float scale,
            const glm::vec3& rotationDegrees = glm::vec3(0.0f)
        );
    private:
        static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.3f);

        Model m_model;
        Shader m_cableCarShader;
        float m_wheelAngle = 0.0f;
};
