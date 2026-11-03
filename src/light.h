#pragma once
#include <glm/glm.hpp>

struct SkyLight{
    glm::vec3 sunDir;
    glm::vec3 sunColor;
    glm::vec3 moonDir;
    glm::vec3 moonColor;
    glm::vec3 ambientColor;
    float cabinLightAmount;
    glm::vec3 cabinLightPos;
    glm::mat4 lightCamera{1.0f};
};
glm::vec3 movingSun(float timeOfDay);
SkyLight computeSkyLight(float timeOfDay);