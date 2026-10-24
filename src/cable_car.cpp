#include <string>
#include "cable_car.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "model.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

CableCar::CableCar(float doorOpenness)
: m_model(Model(static_cast<std::string>(PROJECT_ROOT) + "/assets/cable_car/scene.gltf"))
, m_doorOpenness(doorOpenness)
, m_door_back_left(m_model.getNode("Door_Back_Left"))
, m_door_back_right(m_model.getNode("Door_Back_Right"))
, m_door_front_left(m_model.getNode("Door_Front_Left"))
, m_door_frong_right(m_model.getNode("Door_Front_Right"))
{
    try {
        std::string root = PROJECT_ROOT;
        ShaderBuilder cableCarBuilder;
        cableCarBuilder.addStage(GL_VERTEX_SHADER, root + "/shaders/basic.vert");
        cableCarBuilder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/basic.frag");
        m_cableCarShader = cableCarBuilder.build();

    } catch (const ShaderLoadingException& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
    m_model.setNodeWorldOffset("Door_Back_Left", glm::vec3(0.0f, 0.0f, -0.3f));
    m_model.setNodeWorldOffset("Door_Back_Right", glm::vec3(0.0f, 0.0f, -0.3f));
    m_model.setNodeWorldOffset("Door_Front_Left", glm::vec3(0.0f, 0.0f, 0.3f));
    m_model.setNodeWorldOffset("Door_Front_Right", glm::vec3(0.0f, 0.0f, 0.3f));
};

void CableCar::setDoorOpenness(float openness) {
    m_doorOpenness = openness;
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

    m_model.draw(m_cableCarShader, m_cableCarShader.getUniformLocation("model"), m_doorOpenness);
}
