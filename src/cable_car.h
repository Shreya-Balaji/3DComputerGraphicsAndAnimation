#pragma once

#include "glm/ext/vector_float3.hpp"
#include "model.h"

class CableCar {
    public:
        CableCar();
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition);

    private:
        Model m_model;
        Shader m_cableCarShader;

        bool m_doorsOpen;
};
