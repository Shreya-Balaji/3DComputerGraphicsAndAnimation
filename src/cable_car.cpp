#include "cable_car.h"

#include <assimp/material.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "light.h"
#include "environment_map.h"
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
glm::vec3 evaluateCableCarRoute(float t)
{
    const glm::vec3 start(-5.4f, 0.0f, -2.0f);
    const glm::vec3 end(-5.4f, 0.4f, -8.0f);

    t = glm::clamp(t, 0.0f, 1.0f);

    return glm::mix(start, end, t);
}
void bindLighting(const Shader& shader, const glm::mat4& viewProjection,
                  const glm::vec3& cameraPosition, const SkyLight& light, const Config& config)
{
    shader.bind();
    glUniformMatrix4fv(shader.getUniformLocation("viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform3fv(shader.getUniformLocation("viewPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(shader.getUniformLocation("lightDir"), 1, glm::value_ptr(light.sunDir));
    glUniform3fv(shader.getUniformLocation("sunColor"), 1, glm::value_ptr(light.sunColor));
    glUniform3fv(shader.getUniformLocation("moonDir"), 1, glm::value_ptr(light.moonDir));
    glUniform3fv(shader.getUniformLocation("moonColor"), 1, glm::value_ptr(light.moonColor));
    glUniform3fv(shader.getUniformLocation("ambientColor"), 1, glm::value_ptr(light.ambientColor));
    glUniform3fv(shader.getUniformLocation("cabinLightPos"), 1, glm::value_ptr(light.cabinLightPos));
    glUniform3fv(shader.getUniformLocation("cabinLightColor"), 1, glm::value_ptr(config.cabinLightColor));
    glUniform1f(shader.getUniformLocation("cabinLightAmount"), light.cabinLightAmount);
    glUniform1f(shader.getUniformLocation("cabinLightRadius"), config.cabinLightRadius);
    glUniform1f(shader.getUniformLocation("cabinLightIntensity"), config.cabinLightIntensity);
}
}
void CableCar::createCable()
{
    Mesh cable;

    cable.material.kd = glm::vec3(0.12f, 0.12f, 0.14f);
    cable.material.ks = glm::vec3(0.5f);
    cable.material.shininess = 64.0f;
    cable.material.transparency = 1.0f;

    constexpr int segments = 128;
    constexpr int radialSegments = 8;
    constexpr float radius = 0.0095f;
    constexpr float cableLength = 64.0f;


    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) / segments;

        glm::vec3 center(
            -5.4f,
            0.92f,
            -cableLength * 0.5f + t * cableLength
        );

        glm::vec3 tangent(0.0f, 0.0f, 1.0f);
        glm::vec3 referenceUp(0.0f, 1.0f, 0.0f);

        glm::vec3 side = glm::normalize(
            glm::cross(tangent, referenceUp));

        glm::vec3 up = glm::normalize(
            glm::cross(side, tangent));

        for (int j = 0; j < radialSegments; ++j)
        {
            float angle =
                glm::two_pi<float>() * j / radialSegments;

            glm::vec3 normal =
                glm::cos(angle) * side +
                glm::sin(angle) * up;

            Vertex vertex;
            vertex.position = center + radius * normal;
            vertex.normal = normal;
            vertex.texCoord = glm::vec2(
                static_cast<float>(i) / segments,
                static_cast<float>(j) / radialSegments
            );
            vertex.tangent = glm::vec4(tangent, 1.0f);

            cable.vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < segments; ++i)
    {
        for (int j = 0; j < radialSegments; ++j)
        {
            unsigned int current =
                i * radialSegments + j;

            unsigned int next =
                i * radialSegments +
                (j + 1) % radialSegments;

            unsigned int nextRing =
                (i + 1) * radialSegments + j;

            unsigned int nextRingNext =
                (i + 1) * radialSegments +
                (j + 1) % radialSegments;

            cable.triangles.emplace_back(
                current, next, nextRing);

            cable.triangles.emplace_back(
                next, nextRingNext, nextRing);
        }
    }

    m_cableMesh = std::make_unique<GPUMesh>(cable);
}

CableCar::CableCar(const Config& config)
: m_config(config)
, m_model(Model(std::string(PROJECT_ROOT) + "/assets/cable_car/scene.gltf"))
{
    const aiScene* scene = m_model.getScene();
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiString materialName;
        const unsigned int materialIndex = scene->mMeshes[i]->mMaterialIndex;
        if (materialIndex < scene->mNumMaterials)
            scene->mMaterials[materialIndex]->Get(AI_MATKEY_NAME, materialName);
        if (std::string(materialName.C_Str()) == "Windows")
            m_windowMeshes.push_back(i);
        else
            m_bodyMeshes.push_back(i);
    }

    try {
        ShaderBuilder cableCarBuilder;
        cableCarBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        cableCarBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.frag");
        m_cableCarShader = cableCarBuilder.build();
        ShaderBuilder pbrBuilder;
        pbrBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        pbrBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/pbr.frag");
        m_pbrShader = pbrBuilder.build();
        ShaderBuilder windowBuilder;
        windowBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        windowBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/window.frag");
        m_windowShader = windowBuilder.build();


    } catch (const ShaderLoadingException& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
    createCable();
}

glm::vec3 CableCar::position() const
{
    return position(m_terrainOffset);
}

glm::vec3 CableCar::position(float terrainOffset) const
{
    glm::vec3 result = evaluateCableCarRoute(1.0f - m_config.cableCarT);
    result.z += terrainOffset;
    return result;
}

void CableCar::update(float deltaTime, float terrainOffset)
{
    m_terrainOffset = terrainOffset;

    m_model.setWorldTransform(position(), 0.55f);

    m_model.setNodeWorldOffset("Door_Back_Left", -OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Back_Right", -OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Front_Left", OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Front_Right", OPEN_DOOR * m_config.doorSlideFactor);

    m_wheelAngle = std::fmod(
        m_wheelAngle + glm::radians(m_config.wheelSpeed) * deltaTime,
        glm::two_pi<float>());

    for (int i = 1; i <= 8; ++i)
        m_model.setNodeRotation(
            "Wheel_" + std::to_string(i),
            m_wheelAngle,
            glm::vec3(1.0f, 0.0f, 0.0f));
}

void CableCar::draw(const glm::mat4 viewProjection, const glm::vec3 cameraPosition, const SkyLight& light, bool usePbr, bool isMinimap) const {
    if (m_cableMesh)
    {
        m_cableCarShader.bind();

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

        glUniform3fv(m_cableCarShader.getUniformLocation("lightDir"), 1,
            glm::value_ptr(light.sunDir));

        glUniform3fv(m_cableCarShader.getUniformLocation("sunColor"), 1,
            glm::value_ptr(light.sunColor));

        glUniform3fv(m_cableCarShader.getUniformLocation("moonDir"), 1,
            glm::value_ptr(light.moonDir));

        glUniform3fv(m_cableCarShader.getUniformLocation("moonColor"), 1,
            glm::value_ptr(light.moonColor));

        glUniform3fv(m_cableCarShader.getUniformLocation("ambientColor"), 1,
            glm::value_ptr(light.ambientColor));

        constexpr float cableTileSize = 64.0f;

        for (int z = -1; z <= 1; ++z)
        {
            glm::mat4 cableModel = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(
                    0.0f,
                    0.0f,
                    m_terrainOffset + z * cableTileSize
                )
            );
            if (isMinimap)
            {
                cableModel = glm::translate(
                    cableModel,
                    glm::vec3(-5.4f, 0.92f, 0.0f)
                );

                cableModel = glm::scale(
                    cableModel,
                    glm::vec3(5.0f, 5.0f, 1.0f)
                );

                cableModel = glm::translate(
                    cableModel,
                    glm::vec3(5.4f, -0.92f, 0.0f)
                );
            }

            glUniformMatrix4fv(m_cableCarShader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(cableModel));
            m_cableMesh->draw(m_cableCarShader);
        }
    };
    drawBody(viewProjection, cameraPosition, light, usePbr);
}

void CableCar::drawBody(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                        const SkyLight& light, bool usePbr) const
{
    if (m_bodyMeshes.empty())
        return;

    const Shader& shader = usePbr ? m_pbrShader : m_cableCarShader;
    bindLighting(shader, viewProjection, cameraPosition, light, m_config);
    if (usePbr) {
        glUniform1f(shader.getUniformLocation("metallic"), m_config.pbrMetallic);
        glUniform1f(shader.getUniformLocation("roughness"), m_config.pbrRoughness);
    }
    m_model.draw(shader, m_bodyMeshes);
}

void CableCar::drawWindows(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                           const SkyLight& light) const
{
    if (m_windowMeshes.empty())
        return;

    const GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint sourceRGB, destinationRGB, sourceAlpha, destinationAlpha;
    glGetIntegerv(GL_BLEND_SRC_RGB, &sourceRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &destinationRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &sourceAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &destinationAlpha);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    bindLighting(m_windowShader, viewProjection, cameraPosition, light, m_config);
    glUniform1i(m_windowShader.getUniformLocation("envMap"), 2);
    glUniform1f(m_windowShader.getUniformLocation("envMaxLod"), EnvironmentMap::MAX_LOD);
    glUniform1i(m_windowShader.getUniformLocation("useEnvMap"), m_config.useEnvMap);
    m_model.draw(m_windowShader, m_windowMeshes);

    glBlendFuncSeparate(sourceRGB, destinationRGB, sourceAlpha, destinationAlpha);
    if (!blendEnabled)
        glDisable(GL_BLEND);
}
