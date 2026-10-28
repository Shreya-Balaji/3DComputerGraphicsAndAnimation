#pragma once

#include "model.h"

class CableCar {
    public:
        CableCar();
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition, const glm::vec3 sunLightDir) const;
        void setDoorSlideFactor(float slideFactor);
        void setWorldTransform(
            const glm::vec3& position,
            float scale,
            const glm::vec3& rotationDegrees = glm::vec3(0.0f)
        );
    private:
        static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.3f);

        Model m_model;
        Shader m_cableCarShader;
};
