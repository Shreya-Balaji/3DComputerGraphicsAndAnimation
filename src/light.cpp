#include <glm/glm.hpp>
#include <vector>
#include "light.h"

glm::vec3 movingSun(float timeOfDay){
    const float angle = glm::radians(2*180.0f * timeOfDay);
    glm::vec3 sunPos = glm::vec3(glm::cos(angle), glm::sin(angle), 0.5f); // I dont want the path of sun's motion to be straight, so I'm adding some movement along z axis as well to tilt a bit.
    return glm::normalize(-sunPos); // I want sunlight on the cable car and view, so im adding a minus sign to make lightdir point at the scene
}

SkyLight computeSkyLight(float timeOfDay)
{
    SkyLight light;

    light.sunDir = movingSun(timeOfDay);
    const float h = -light.sunDir.y;

    const glm::vec3 day(1.0f, 1.0f, 0.863f);         
    const glm::vec3 dusk(1.0f, 0.235f, 0.235f);       
    const glm::vec3 moonBlue(0.078f, 0.235f, 0.706f); 
    const glm::vec3 nightAmbient(0.08f, 0.10f, 0.18f);
    const glm::vec3 dayAmbient(0.35f);
    light.cabinLightAmount = 1.0f - glm::smoothstep(-0.1f, 0.2f, h);
    light.sunColor = glm::mix(dusk, day, glm::smoothstep(0.0f, 0.3f, h))
                   * glm::smoothstep(-0.1f, 0.05f, h);
    light.moonDir = -light.sunDir;
    light.moonColor = moonBlue * 0.9f * glm::smoothstep(0.0f, 0.2f, -h);
    light.ambientColor = glm::mix(nightAmbient, dayAmbient,
                                  glm::smoothstep(-0.2f, 0.2f, h));
    return light;
}