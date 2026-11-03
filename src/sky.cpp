#include "sky.h"

glm::vec3 computeSkyColor(const SkyLight& light, const Config& config){
    const float h = -light.sunDir.y;    
    const float dayAmount = glm::smoothstep(0.0f, 0.5f, h);
    const float lightRemaining  = glm::smoothstep(-0.15f, 0.0f, h);
    const glm::vec3 skyColor = glm::mix(config.nightColor, glm::mix(config.duskColor, config.dayColor, dayAmount), lightRemaining);
    return skyColor;
}