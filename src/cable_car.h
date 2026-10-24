#pragma once

#include "glm/ext/vector_float3.hpp"
#include "model.h"

class CableCar {
    public:
        CableCar(float doorOpenness);
        void draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition) const;
        void setDoorOpenness(float openness);
    private:
        Model m_model;
        Shader m_cableCarShader;

        const aiNode* m_door_back_left = nullptr;
        const aiNode* m_door_back_right = nullptr;
        const aiNode* m_door_front_left = nullptr;
        const aiNode* m_door_frong_right = nullptr;
        bool m_doorsOpen;
        float m_doorOpenness = 0.0f;
};
