#pragma once

#include "glm/ext/vector_float3.hpp"
#include "model.h"

class CableCar {
    public:
        CableCar();
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition) const;
        void setDoorSlideFactor(float slideFactor);
    private:
        static constexpr glm::vec3 OPEN_DOOR = glm::vec3(0.0f, 0.0f, 0.3f);

        Model m_model;
        Shader m_cableCarShader;
};
