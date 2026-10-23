#pragma once

#include "glm/ext/vector_float3.hpp"
#include "model.h"

class CableCar {
    public:
        CableCar();
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition) const;

    private:
        Model m_model;
        Shader m_cableCarShader;

        const aiNode* m_doors = nullptr;

        bool m_doorsOpen;
};
