#include "cable_car.h"

#include <assimp/Importer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>

CableCar::CableCar()
: m_model(Model(std::string(PROJECT_ROOT) + "/assets/cable_car/scene.gltf"))
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

void CableCar::setDoorSlideFactor(float slideFactor) {
    m_model.setNodeWorldOffset("Door_Back_Left", -OPEN_DOOR * slideFactor);
    m_model.setNodeWorldOffset("Door_Back_Right", -OPEN_DOOR * slideFactor);
    m_model.setNodeWorldOffset("Door_Front_Left", OPEN_DOOR * slideFactor);
    m_model.setNodeWorldOffset("Door_Front_Right", OPEN_DOOR * slideFactor);
}

void CableCar::setWorldTransform(
    const glm::vec3& position,
    float scale,
    const glm::vec3& rotationDegrees)
{
    m_model.setWorldTransform(position, scale, rotationDegrees);
}

void CableCar::draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition, const glm::vec3 sunLightDir) const {
    m_cableCarShader.bind();

    glUniform3f(
       m_cableCarShader.getUniformLocation("lightDir"),
        sunLightDir.x, sunLightDir.y, sunLightDir.z
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
