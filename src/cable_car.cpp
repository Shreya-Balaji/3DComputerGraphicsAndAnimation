#include "cable_car.h"

#include <assimp/Importer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <cmath>
#include <iostream>
#include <string>

namespace {
glm::vec3 evaluateCableCarRoute(float t)
{
    // Route along the outside edge of the mountain.
    const glm::vec3 p0(-5.4f, 0.0f, -2.0f);
    const glm::vec3 p1(-5.7f, 0.7f, -3.8f);
    const glm::vec3 p2(-5.7f, 1.0f, -6.0f);
    const glm::vec3 p3(-5.4f, 0.4f, -8.0f);

    t = glm::clamp(t, 0.0f, 1.0f);
    const float u = 1.0f - t;
    return u * u * u * p0
        + 3.0f * u * u * t * p1
        + 3.0f * u * t * t * p2
        + t * t * t * p3;
}
}

CableCar::CableCar(const Config& config)
: m_config(config)
, m_model(Model(std::string(PROJECT_ROOT) + "/assets/cable_car/scene.gltf"))
{
    try {
        ShaderBuilder cableCarBuilder;
        cableCarBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        cableCarBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.frag");
        m_cableCarShader = cableCarBuilder.build();

    } catch (const ShaderLoadingException& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
};

void CableCar::update(float deltaTime)
{
    m_model.setWorldTransform(evaluateCableCarRoute(1.0f - m_config.cableCarT), 0.55f);
    m_model.setNodeWorldOffset("Door_Back_Left", -OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Back_Right", -OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Front_Left", OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Front_Right", OPEN_DOOR * m_config.doorSlideFactor);

    m_wheelAngle = std::fmod(
        m_wheelAngle + glm::radians(m_config.wheelSpeed) * deltaTime,
        glm::two_pi<float>());

    for (int i = 1; i <= 8; ++i)
        m_model.setNodeRotation("Wheel_" + std::to_string(i), m_wheelAngle, glm::vec3(1.0f, 0.0f, 0.0f));
}

void CableCar::draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition) const {
    m_cableCarShader.bind();

    glUniform3f(
       m_cableCarShader.getUniformLocation("lightDir"),
        -0.5f, -1.0f, -0.3f
    );
    glUniformMatrix4fv(
        m_cableCarShader.getUniformLocation("viewProjection"),
        1,
        GL_FALSE,
        glm::value_ptr(viewProjection)
    );

    glUniform3fv(
        m_cableCarShader.getUniformLocation("viewPos"),
        1,
        glm::value_ptr(cameraPosition)
    );

    m_model.draw(m_cableCarShader);
}
